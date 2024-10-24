# - Find compton library
# This module sets up compton information
# It defines:
# REMOLL_FOUND               If the compton is found
# REMOLL_INCLUDE_DIR         PATH to the include directory
# REMOLL_LIBRARY_DIR         PATH to the library directory

get_filename_component(_thisdir "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_compton "${_thisdir}/../../.." ABSOLUTE)


find_program(REMOLL_CONFIG NAMES Remoll-config
             PATHS $ENV{REMOLL_INSTALL}/bin
                   ${REMOLL_INSTALL}/bin
                   /usr/local/bin /opt/local/bin)

if(REMOLL_CONFIG)
  set(REMOLL_FOUND TRUE)

  execute_process(COMMAND ${REMOLL_CONFIG} --prefix
                  OUTPUT_VARIABLE REMOLL_PREFIX
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(COMMAND ${REMOLL_CONFIG} --bindir
                  OUTPUT_VARIABLE REMOLL_BINARY_DIR
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(COMMAND ${REMOLL_CONFIG} --incdir
                  OUTPUT_VARIABLE REMOLL_INCLUDE_DIR
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(COMMAND ${REMOLL_CONFIG} --libdir
                  OUTPUT_VARIABLE REMOLL_LIBRARY_DIR
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(COMMAND ${REMOLL_CONFIG} --libs
                  OUTPUT_VARIABLE REMOLL_LIBRARIES
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

  message(STATUS "Found compton: ${REMOLL_PREFIX}")

else()
  set(REMOLL_FOUND FALSE)
  message(STATUS "Not found compton: set REMOLL_INSTALL env var.")

  message(STATUS "Setting directories relative to cmake file...")
  set(REMOLL_BINARY_DIR ${_compton}/bin)
  set(REMOLL_INCLUDE_DIR ${_compton}/include)
  set(REMOLL_LIBRARY_DIR ${_compton}/${CMAKE_INSTALL_LIBDIR})
  set(REMOLL_LIBRARIES "-L${CMAKE_INSTALL_LIBDIR} -lcompton")

endif()

set(REMOLL_USE_FILE "${_thisdir}/RemollUseFile.cmake")
