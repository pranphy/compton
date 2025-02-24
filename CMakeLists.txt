#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 3.8)
project(compton VERSION 0.2 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
# Export compile commands as json for run-clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Also use clang-tidy integration in CMake
option(ENABLE_CLANG_TIDY "Enable clang-tidy integration in cmake" OFF)
if(ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
  if (CLANG_TIDY_EXE)
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}" CACHE STRING "" FORCE)
  else()
    set(CMAKE_CXX_CLANG_TIDY "" CACHE STRING "" FORCE)
  endif()
endif()

# Disallow in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "compton requires an out-of-source build.")
endif()

# Prepend module search path
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

# Use our modules and utility functions
include(BuildType)
include(PrefixDir)
include(CMakeEnv)

# Use GNU install dirs
include(GNUInstallDirs)

# MAC specific variable
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
    if("${isSystemDir}" STREQUAL "-1")
        set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    endif()
endif()

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
file(GLOB sources src/compton*.cc)
file(GLOB headers include/compton*.hh)
find_package(XercesC 3.2.0 REQUIRED)


#----------------------------------------------------------------------------
# ROOT
#
set(minimum_root_version 6.0)
find_package(ROOT ${minimum_root_version} REQUIRED)
config_add_dependency(ROOT ${minimum_root_version})


#----------------------------------------------------------------------------
# gitinfo.cc
#
set(PRE_CONFIGURE_FILE ${PROJECT_SOURCE_DIR}/cmake/templates/gitinfo.cc.in)
set(POST_CONFIGURE_FILE ${CMAKE_BINARY_DIR}/gitinfo.cc)
include(CMakeGitInfo)
list(APPEND sources ${POST_CONFIGURE_FILE})


#----------------------------------------------------------------------------
# main dictionaries
#
message(STATUS "Generating ${PROJECT_NAME} dictionaries for ROOT ${ROOT_VERSION}...")
foreach(file ${headers})
  # Find the header files that have uncommented ClassDef lines
  execute_process(COMMAND grep -l "[^/]ClassDef" ${file} OUTPUT_VARIABLE result)
  if(result)
    list(APPEND needsdict ${file})
  endif()
endforeach()
build_root_dictionary(
  ${PROJECT_NAME}
  ${needsdict}
  TARGETS lib${PROJECT_NAME}
  LINKDEF ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}LinkDef.hh
)
list(APPEND sources ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Dict.cxx)



#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 4.10 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 4.10 REQUIRED)
endif()

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
# Setup include directory for this project
#
include(${Geant4_USE_FILE})


#----------------------------------------------------------------------------
# Compiler flags
#
# Set compiler flags to the set given by ROOT set
set_compiler_flags("${ROOT_CXX_FLAGS}")
# FIXME: The following will at some point all disappear (yeah yeah, soon)
# Ignore shadowing of CLHEP 1-letter units with 1-letter local variables
string(APPEND ${PROJECT_NAME_UC}_DIAG_FLAGS " -Wno-shadow")
# Ignore missing field initializers in structs
string(APPEND ${PROJECT_NAME_UC}_DIAG_FLAGS " -Wno-missing-field-initializers")
# Ignore unused parameters in many geant4 functions
string(APPEND ${PROJECT_NAME_UC}_DIAG_FLAGS " -Wno-unused-parameter")
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
  # Ignore warning of struct initialization { } for gcc < 5.0 (after that it is ignored internally)
  string(APPEND ${PROJECT_NAME_UC}_DIAG_FLAGS " -Wno-missing-field-initializers")
endif()
# Now add the standard WALL and WEXTRA sets
set_diagnostic_flags(WALL WEXTRA)
# Finally print what we use
report_build_info()


#----------------------------------------------------------------------------
# main library
#
add_library(lib${PROJECT_NAME} SHARED ${sources} ${headers})
add_dependencies(lib${PROJECT_NAME} check_git_repository)
set_target_properties(lib${PROJECT_NAME} PROPERTIES OUTPUT_NAME compton)

target_include_directories(lib${PROJECT_NAME}
  PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  )

target_compile_options(lib${PROJECT_NAME}
  PUBLIC
    ${${PROJECT_NAME_UC}_CXX_FLAGS_LIST}
  PRIVATE
    ${${PROJECT_NAME_UC}_DIAG_FLAGS_LIST}
  )

target_link_libraries(lib${PROJECT_NAME}
  PUBLIC
    ROOT::Libraries
    ${Geant4_LIBRARIES}
    XercesC::XercesC
  )

install(TARGETS lib${PROJECT_NAME}
  EXPORT ${MAIN_PROJECT_NAME_LC}-exports
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
install(FILES ${headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})


#----------------------------------------------------------------------------
# executables
#
file(GLOB exefiles *.cc)
foreach(file ${exefiles})
  get_filename_component(filename ${file} NAME_WE)
  string(TOLOWER ${filename} filelower)

  add_executable(${filelower} ${file})

  target_link_libraries(${filelower}
    PRIVATE
      lib${PROJECT_NAME}
  )
  target_compile_options(${filelower}
    PUBLIC
      ${${PROJECT_NAME_UC}_CXX_FLAGS_LIST}
    PRIVATE
      ${${PROJECT_NAME_UC}_DIAG_FLAGS_LIST}
  )
  if(${CMAKE_SYSTEM_NAME} MATCHES Linux)
    target_compile_options(${filelower} PUBLIC -fPIC)
  endif()

  install(TARGETS ${filelower}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
endforeach()

install(DIRECTORY geometry DESTINATION ${CMAKE_INSTALL_DATADIR}/compton)
install(DIRECTORY macros DESTINATION ${CMAKE_INSTALL_DATADIR}/compton)
#install(DIRECTORY vis DESTINATION ${CMAKE_INSTALL_DATADIR}/compton)

foreach(_script ${COMPTON_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()


#----------------------------------------------------------------------------
# Download field maps subdirectory
#
set(COMPTON_DOWNLOADS http://hallaweb.jlab.org/12GeV/Moller/downloads/compton/)
set(COMPTON_MAP_DIR ${PROJECT_SOURCE_DIR}/map_directory)

set(COMPTON_MAPS
    dipole_magnetic_field_19_1.txt
    dipole_magnetic_field_19_2.txt
    dipole_magnetic_field_19_3.txt
    dipole_magnetic_field_19_4.txt
)

foreach(FILE IN LISTS COMPTON_MAPS)
    message(STATUS "Downloading ${FILE}")
    file(DOWNLOAD
      ${COMPTON_DOWNLOADS}/${FILE}
      ${COMPTON_MAP_DIR}/${FILE}
      )
    install(FILES
      ${COMPTON_MAP_DIR}/${FILE}
      DESTINATION ${CMAKE_INSTALL_DATADIR}/compton/map_directory)
endforeach()

add_definitions(
        -DCMAKE_INSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\"
        -DCMAKE_INSTALL_FULL_DATADIR=\"${CMAKE_INSTALL_FULL_DATADIR}\"
)


# uninstall
if(NOT TARGET uninstall)
  configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/scripts/cmake_uninstall.cmake.in"
      "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
      IMMEDIATE @ONLY)
  add_custom_target(uninstall
      COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
