# Minimum required CMake version. 3.15+ enables modern policies and features.
cmake_minimum_required(VERSION 3.15)
project(compton VERSION 0.3 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Enforce the standard, fail if compiler doesn't support
set(CMAKE_CXX_EXTENSIONS OFF)      # Disallow compiler-specific extensions for portability
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

# Disallow in-source builds for a clean separation of source and build files.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Compton requires an out-of-source build. Please create a 'build' directory (e.g., 'mkdir build') and run cmake from there (e.g., 'cd build && cmake ..').")
endif()

# macOS specific RPATH handling for shared libraries and executables.
if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(CMAKE_MACOSX_RPATH ON) # Embed rpath in binaries
    set(CMAKE_SKIP_BUILD_RPATH FALSE) # Don't skip rpath for build tree
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) # Don't build with install rpath
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) # Use link path for install rpath
    # Ensure install RPATH includes lib directory for installed libraries
    list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" _isSystemDir)
    if("${_isSystemDir}" STREQUAL "-1")
        # Only set if it's not an implicit system directory (to avoid redundant rpaths)
        set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
    endif()
endif()

file(GLOB sources "src/compton*.cc")
file(GLOB headers "include/compton*.hh")

find_package(XercesC 3.2.0 REQUIRED)
find_package(ROOT 6.36 COMPONENTS RIO Core Tree CONFIG REQUIRED)

find_package(Geant4 11.2 REQUIRED ui_all vis_all )
include_directories(${Geant4_INCLUDE_DIRS})

set(GITINFO_CC_OUT "${CMAKE_CURRENT_BINARY_DIR}/gitinfo.cc")
set(GITINFO_CC_IN "${PROJECT_SOURCE_DIR}/cmake/templates/gitinfo.cc.in")

# makes all include headers publicly available
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

find_program(GIT_EXECUTABLE NAMES git)
if(GIT_EXECUTABLE)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" log -1
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_LOG
    )
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" status --short --porcelain
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_STATUS
    )

else()
    message(STATUS "Git executable not found. Using default Git info.")
endif()

configure_file( "${GITINFO_CC_IN}" "${GITINFO_CC_OUT}" @ONLY) # Ensures only @VAR@ style variables are substituted, preventing accidental substitution

list(APPEND sources "${GITINFO_CC_OUT}")

message(STATUS "Generating ${PROJECT_NAME} dictionaries for ROOT ${ROOT_VERSION}...")


foreach(file ${headers})
  # Find the header files that have uncommented ClassDef lines
  execute_process(COMMAND grep -l "[^/]ClassDef" ${file} OUTPUT_VARIABLE result)
  if(result)
    list(APPEND needsdict ${file})
  endif()
endforeach()

list(APPEND needsdict "${PROJECT_SOURCE_DIR}/include/comptontypes.hh")

ROOT_GENERATE_DICTIONARY(
    G__${PROJECT_NAME}# Name of the dictionary target (e.g., G__compton or comptonDict)
    ${needsdict}          # List of header files to scan for ROOT classes
    MODULE ${PROJECT_NAME}
    LINKDEF "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}LinkDef.hh" # Path to the LinkDef file
    OPTIONS -I${Geant4_INCLUDE_DIRS} -I${CLHEP_INCLUDE_DIRS} -I${CMAKE_CURRENT_SOURCE_DIR}/include
)

list(APPEND sources "${CMAKE_CURRENT_BINARY_DIR}/G__${PROJECT_NAME}.cxx")

#
add_library(lib${PROJECT_NAME} SHARED ${sources} ${headers})
set_target_properties(lib${PROJECT_NAME} PROPERTIES OUTPUT_NAME compton)

# Define include directories for the library.
target_include_directories(lib${PROJECT_NAME}
    PUBLIC
        # For projects installing and using this library
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        # For projects building and using this library in the build tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        # For generated headers/sources that are only internal to this build
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
)

# Apply compile options to the library.
target_compile_options(lib${PROJECT_NAME}
    PUBLIC
        ${${PROJECT_NAME_UC}_CXX_FLAGS_LIST} # Common CXX flags
    PRIVATE
        ${${PROJECT_NAME_UC}_DIAG_FLAGS} # Specific diagnostic suppressions
)

# Link necessary libraries.
target_link_libraries(lib${PROJECT_NAME}
    PUBLIC
        ROOT::RIO
        ROOT::Tree
        ROOT::Rint
        ${Geant4_LIBRARIES} # Geant4 libraries (from Geant4_USE_FILE)
        XercesC::XercesC # Modern imported target for XercesC
)

# ----------------------------------------------------------------------------
# Executables
file(GLOB exefiles "*.cc")
foreach(file ${exefiles})
    get_filename_component(filename_we "${file}" NAME_WE) # Get filename without extension
    string(TOLOWER "${filename_we}" filename_lower) # Convert to lowercase for executable name
    add_executable("${filename_lower}" "${file}")
    #target_include_directories(${filename_lower} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries("${filename_lower}" PRIVATE lib${PROJECT_NAME})
    target_compile_options("${filename_lower}"
        PUBLIC
            ${${PROJECT_NAME_UC}_CXX_FLAGS_LIST}
        PRIVATE
            ${${PROJECT_NAME_UC}_DIAG_FLAGS}
    )
endforeach()


# ----------------------------------------------------------------------------
# Download field maps subdirectory
set(COMPTON_DOWNLOADS "http://userweb.jlab.org/~pgautam/downloads/compton")
set(FIELD_ID 20) # Current field map ID
set(COMPTON_MAP_DIR "${PROJECT_SOURCE_DIR}/map_directory/${FIELD_ID}")

set(COMPTON_MAPS
    "dipole_magnetic_field_${FIELD_ID}_1.txt"
    "dipole_magnetic_field_${FIELD_ID}_2.txt"
    "dipole_magnetic_field_${FIELD_ID}_3.txt"
    "dipole_magnetic_field_${FIELD_ID}_4.txt"
)

foreach(FILE IN LISTS COMPTON_MAPS)
    set(LOCAL_FILE_PATH "${COMPTON_MAP_DIR}/${FILE}")
    if(NOT EXISTS "${LOCAL_FILE_PATH}")
        file(MAKE_DIRECTORY "${COMPTON_MAP_DIR}") # Ensure the local directory exists
        message(STATUS "Downloading ${COMPTON_DOWNLOADS}/${FILE} to ${LOCAL_FILE_PATH}")
        file(DOWNLOAD
            "${COMPTON_DOWNLOADS}/${FILE}"
            "${LOCAL_FILE_PATH}"
        )
    else()
        message(STATUS "File already exists: ${LOCAL_FILE_PATH}. Skipping download.")
    endif()
endforeach()

target_compile_definitions(lib${PROJECT_NAME}
    PUBLIC
        CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
        CMAKE_INSTALL_FULL_DATADIR="${CMAKE_INSTALL_FULL_DATADIR}"
)
# Apply these definitions to all executables as well.
foreach(file ${exefiles})
    get_filename_component(filename_we "${file}" NAME_WE)
    string(TOLOWER "${filename_we}" filename_lower)
    target_compile_definitions("${filename_lower}"
        PUBLIC
            CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
            CMAKE_INSTALL_FULL_DATADIR="${CMAKE_INSTALL_FULL_DATADIR}"
    )
endforeach()

