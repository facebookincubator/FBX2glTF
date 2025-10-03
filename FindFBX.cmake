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
  set(FBXSDK_VERSION "2020.3.7")
endif()

set(_fbxsdk_vstudio_version "vs2022")

message("Looking for FBX SDK version: ${FBXSDK_VERSION}")

message(STATUS "FBXSDK_SDKS specified: ${FBXSDK_SDKS}")

if (NOT DEFINED FBXSDK_SDKS)
   set(FBXSDK_SDKS "${CMAKE_CURRENT_SOURCE_DIR}/sdk")
endif()

get_filename_component(FBXSDK_SDKS_ABS ${FBXSDK_SDKS} ABSOLUTE)

set(FBXSDK_APPLE_ROOT   "${FBXSDK_SDKS_ABS}/Darwin/${FBXSDK_VERSION}")
set(FBXSDK_LINUX_ROOT   "${FBXSDK_SDKS_ABS}/Linux/${FBXSDK_VERSION}")
set(FBXSDK_WINDOWS_ROOT "${FBXSDK_SDKS_ABS}")

if (APPLE)
  message(STATUS "Apple: Using FBX SDK from ${FBXSDK_APPLE_ROOT}")
  set(_fbxsdk_root "${FBXSDK_APPLE_ROOT}")
  set(_fbxsdk_libdir_debug "lib/clang/debug")
  set(_fbxsdk_libdir_release "lib/clang/release")
  set(_fbxsdk_libname_debug "libfbxsdk.a")
  set(_fbxsdk_libname_release "libfbxsdk.a")
elseif (WIN32)
  message(STATUS "Windows: Using FBX SDK from ${FBXSDK_WINDOWS_ROOT}")
  set(_fbxsdk_root "${FBXSDK_WINDOWS_ROOT}")
  set(_fbxsdk_libdir_debug "lib/x64/debug")
  set(_fbxsdk_libdir_release "lib/x64/release")
  set(_fbxsdk_libname_debug "libfbxsdk-md.lib")
  set(_fbxsdk_libxml2_libname_debug "libxml2-md.lib")
  set(_fbxsdk_alembic_libname_debug "alembic-md.lib")
  set(_fbxsdk_zlib_libname_debug "zlib-md.lib")
  set(_fbxsdk_libname_release "libfbxsdk-md.lib")
  set(_fbxsdk_libxml2_libname_release "libxml2-md.lib")
  set(_fbxsdk_alembic_libname_release "alembic-md.lib")
  set(_fbxsdk_zlib_libname_release "zlib-md.lib")
elseif (LINUX)
  message(STATUS "Linux: Using FBX SDK from ${FBXSDK_LINUX_ROOT}")
  set(_fbxsdk_root "${FBXSDK_LINUX_ROOT}")
  set(_fbxsdk_libdir_debug "lib/debug")
  set(_fbxsdk_libdir_release "lib/release")
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

if (MSVC)
# libalembic-md.lib
find_library(FBXSDK_ALEMBIC_LIBRARY
  ${_fbxsdk_alembic_libname_release}
  NO_CMAKE_FIND_ROOT_PATH
  PATHS "${FBXSDK_ROOT}/${_fbxsdk_libdir_release}")
message("FBXSDK_ALEMBIC_LIBRARY: ${FBXSDK_ALEMBIC_LIBRARY}")

# libalembic.lib debug
find_library(FBXSDK_ALEMBIC_LIBRARY_DEBUG
  NAMES ${_fbxsdk_alembic_libname_debug}
  NO_CMAKE_FIND_ROOT_PATH
  PATHS "${FBXSDK_ROOT}/${_fbxsdk_libdir_debug}")
message("FBXSDK_ALEMBIC_LIBRARY_DEBUG: ${FBXSDK_ALEMBIC_LIBRARY_DEBUG}")

# libxml2-md.lib
find_library(FBXSDK_LIBXML2_LIBRARY
  ${_fbxsdk_libxml2_libname_release}
  NO_CMAKE_FIND_ROOT_PATH
  PATHS "${FBXSDK_ROOT}/${_fbxsdk_libdir_release}")
message("FBXSDK_LIBXML2_LIBRARY: ${FBXSDK_LIBXML2_LIBRARY}")

# libxml2-md.lib debug
find_library(FBXSDK_LIBXML2_LIBRARY_DEBUG
${_fbxsdk_libxml2_libname_debug}
  NO_CMAKE_FIND_ROOT_PATH
  PATHS "${FBXSDK_ROOT}/${_fbxsdk_libdir_debug}")
message("FBXSDK_LIBXML2_LIBRARY_DEBUG: ${FBXSDK_LIBXML2_LIBRARY_DEBUG}")

# zlib-md.lib
find_library(FBXSDK_ZLIB_LIBRARY
  ${_fbxsdk_zlib_libname_release}
  NO_CMAKE_FIND_ROOT_PATH
  PATHS "${FBXSDK_ROOT}/${_fbxsdk_libdir_release}")
message("FBXSDK_ZLIB_LIBRARY: ${FBXSDK_ZLIB_LIBRARY}")

# zlib-md.lib debug
find_library(FBXSDK_ZLIB_LIBRARY_DEBUG
  ${_fbxsdk_zlib_libname_debug}
  NO_CMAKE_FIND_ROOT_PATH
  PATHS "${FBXSDK_ROOT}/${_fbxsdk_libdir_debug}")
message("FBXSDK_ZLIB_LIBRARY_DEBUG: ${FBXSDK_ZLIB_LIBRARY_DEBUG}")

endif()

find_library(FBXSDK_LIBRARY  ${_fbxsdk_libname_release}
  NO_CMAKE_FIND_ROOT_PATH
  PATHS "${FBXSDK_ROOT}/${_fbxsdk_libdir_release}")
message("FBXSDK_LIBRARY: ${FBXSDK_LIBRARY}")

message("FBXSDK_LIBRARY_DEBUG: ${FBXSDK_ROOT}/${_fbxsdk_libdir_debug}/${_fbxsdk_libname_debug}")
find_library(FBXSDK_LIBRARY_DEBUG ${_fbxsdk_libname_debug}
  NO_CMAKE_FIND_ROOT_PATH
  PATHS "${FBXSDK_ROOT}/${_fbxsdk_libdir_debug}")
message("FBXSDK_LIBRARY_DEBUG: ${FBXSDK_LIBRARY_DEBUG}")

if (FBXSDK_INCLUDE_DIR AND FBXSDK_LIBRARY AND FBXSDK_LIBRARY_DEBUG)
  set(FBXSDK_FOUND YES)
else()
  set(FBXSDK_FOUND NO)
endif()
