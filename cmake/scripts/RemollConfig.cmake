# - Find compton library
# This module sets up compton information
# It defines:
# COMPTON_FOUND               If the compton is found
# COMPTON_INCLUDE_DIR         PATH to the include directory
# COMPTON_LIBRARY_DIR         PATH to the library directory

get_filename_component(_thisdir "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_compton "${_thisdir}/../../.." ABSOLUTE)


find_program(COMPTON_CONFIG NAMES Compton-config
             PATHS $ENV{COMPTON_INSTALL}/bin
                   ${COMPTON_INSTALL}/bin
                   /usr/local/bin /opt/local/bin)

if(COMPTON_CONFIG)
  set(COMPTON_FOUND TRUE)

  execute_process(COMMAND ${COMPTON_CONFIG} --prefix
                  OUTPUT_VARIABLE COMPTON_PREFIX
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(COMMAND ${COMPTON_CONFIG} --bindir
                  OUTPUT_VARIABLE COMPTON_BINARY_DIR
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(COMMAND ${COMPTON_CONFIG} --incdir
                  OUTPUT_VARIABLE COMPTON_INCLUDE_DIR
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(COMMAND ${COMPTON_CONFIG} --libdir
                  OUTPUT_VARIABLE COMPTON_LIBRARY_DIR
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(COMMAND ${COMPTON_CONFIG} --libs
                  OUTPUT_VARIABLE COMPTON_LIBRARIES
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

  message(STATUS "Found compton: ${COMPTON_PREFIX}")

else()
  set(COMPTON_FOUND FALSE)
  message(STATUS "Not found compton: set COMPTON_INSTALL env var.")

  message(STATUS "Setting directories relative to cmake file...")
  set(COMPTON_BINARY_DIR ${_compton}/bin)
  set(COMPTON_INCLUDE_DIR ${_compton}/include)
  set(COMPTON_LIBRARY_DIR ${_compton}/${CMAKE_INSTALL_LIBDIR})
  set(COMPTON_LIBRARIES "-L${CMAKE_INSTALL_LIBDIR} -lcompton")

endif()

set(COMPTON_USE_FILE "${_thisdir}/ComptonUseFile.cmake")
