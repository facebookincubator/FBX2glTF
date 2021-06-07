# Copyright (c) 2014-present, Facebook, Inc.
# All rights reserved.
#
# Helper function for finding the FBX SDK.
# Cribbed & tweaked from https://github.com/floooh/fbxc/
#
# params: FBXSDK_VERSION
#         FBXSDK_SDKS
#
# sets:   FBXSDK_FOUND
#         FBXSDK_DIR
#         FBXSDK_LIBRARY
#         FBXSDK_LIBRARY_DEBUG
#         FBXSDK_INCLUDE_DIR
#

# semi-hack to detect architecture
if( CMAKE_SIZEOF_VOID_P MATCHES 8 )
  # void ptr = 8 byte --> x86_64
  set(ARCH_32 OFF)
else()
  # void ptr != 8 byte --> x86
  set(ARCH_32 OFF)
endif()

if (NOT DEFINED FBXSDK_VERSION)
  set(FBXSDK_VERSION "2020.2")
endif()

set(_fbxsdk_vstudio_version "vs2019")

message("Looking for FBX SDK version: ${FBXSDK_VERSION}")

if (NOT DEFINED FBXSDK_SDKS)
   set(FBXSDK_SDKS "${CMAKE_CURRENT_SOURCE_DIR}/sdk")
endif()

get_filename_component(FBXSDK_SDKS_ABS ${FBXSDK_SDKS} ABSOLUTE)

set(FBXSDK_APPLE_ROOT   "${FBXSDK_SDKS_ABS}/Darwin/${FBXSDK_VERSION}")
set(FBXSDK_LINUX_ROOT   "${FBXSDK_SDKS_ABS}/Linux/${FBXSDK_VERSION}")
set(FBXSDK_WINDOWS_ROOT "${FBXSDK_SDKS_ABS}/Windows/${FBXSDK_VERSION}")

if (APPLE)
  set(_fbxsdk_root "${FBXSDK_APPLE_ROOT}")
  set(_fbxsdk_libdir_debug "lib/clang/debug")
  set(_fbxsdk_libdir_release "lib/clang/release")
  set(_fbxsdk_libname_debug "libfbxsdk.a")
  set(_fbxsdk_libname_release "libfbxsdk.a")
elseif (WIN32)
  set(_fbxsdk_root "${FBXSDK_WINDOWS_ROOT}")
  if (ARCH_32)
    set(_fbxsdk_libdir_debug "lib/${_fbxsdk_vstudio_version}/x86/debug")
    set(_fbxsdk_libdir_release "lib/${_fbxsdk_vstudio_version}/x86/release")
  else()
    set(_fbxsdk_libdir_debug "lib/${_fbxsdk_vstudio_version}/x64/debug")
    set(_fbxsdk_libdir_release "lib/${_fbxsdk_vstudio_version}/x64/release")
  endif()
  set(_fbxsdk_libname_debug "libfbxsdk-md.lib")
  set(_fbxsdk_libname_release "libfbxsdk-md.lib")
elseif (UNIX)
  set(_fbxsdk_root "${FBXSDK_LINUX_ROOT}")
  if (ARCH_32)
    set(_fbxsdk_libdir_debug "lib/gcc/x86/debug")
    set(_fbxsdk_libdir_release "lib/gcc/x86/release")
  else()
    set(_fbxsdk_libdir_debug "lib/gcc/x64/debug")
    set(_fbxsdk_libdir_release "lib/gcc/x64/release")
  endif()
  set(_fbxsdk_libname_debug "libfbxsdk.a")
  set(_fbxsdk_libname_release "libfbxsdk.a")
else()
  message(FATAL_ERROR, "Unknown platform. Can't find FBX SDK.")
endif()

# should point the the FBX SDK installation dir
set(FBXSDK_ROOT "${_fbxsdk_root}")
message("FBXSDK_ROOT: ${FBXSDK_ROOT}")

# find header dir and libs
find_path(FBXSDK_INCLUDE_DIR "fbxsdk.h"
  NO_CMAKE_FIND_ROOT_PATH
  PATHS ${FBXSDK_ROOT}
  PATH_SUFFIXES "include")
message("FBXSDK_INCLUDE_DIR: ${FBXSDK_INCLUDE_DIR}")

find_library(FBXSDK_LIBRARY ${_fbxsdk_libname_release}
  NO_CMAKE_FIND_ROOT_PATH
  PATHS "${FBXSDK_ROOT}/${_fbxsdk_libdir_release}")
message("FBXSDK_LIBRARY: ${FBXSDK_LIBRARY}")

find_library(FBXSDK_LIBRARY_DEBUG ${_fbxsdk_libname_debug}
  NO_CMAKE_FIND_ROOT_PATH
  PATHS "${FBXSDK_ROOT}/${_fbxsdk_libdir_debug}")
message("FBXSDK_LIBRARY_DEBUG: ${FBXSDK_LIBRARY_DEBUG}")

if (FBXSDK_INCLUDE_DIR AND FBXSDK_LIBRARY AND FBXSDK_LIBRARY_DEBUG)
  set(FBXSDK_FOUND YES)
else()
  set(FBXSDK_FOUND NO)
endif()
