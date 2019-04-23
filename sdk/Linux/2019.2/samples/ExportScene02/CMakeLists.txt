SET(CMAKE_MIN_REQ_VERSION 2.4)
cmake_minimum_required(VERSION ${CMAKE_MIN_REQ_VERSION})
INCLUDE("../CMakeSettings.txt")

FBX_PROJECT(ExportScene02 .)
SET(FBX_TARGET_SOURCE
    main.cxx
    ../Common/Common.h
    ../Common/Common.cxx
)
ADD_EXECUTABLE(
   ${FBX_TARGET_NAME}
   ${FBX_TARGET_SOURCE}
)
SET_SAMPLES_GLOBAL_FLAGS()
