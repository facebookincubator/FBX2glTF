set(draco_VERSION "1.3.4")
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was DracoConfig.cmake                            ########
get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()
####################################################################################
set_and_check(draco_INCLUDE_DIR "${PACKAGE_PREFIX_DIR}/include/draco")
set_and_check(draco_LIBRARY_DIR "${PACKAGE_PREFIX_DIR}/lib")
