SET(CMAKE_MIN_REQ_VERSION 2.4)
cmake_minimum_required(VERSION ${CMAKE_MIN_REQ_VERSION})
INCLUDE("../CMakeSettings.txt")

FBX_PROJECT(Audio .)
SET(FBX_TARGET_SOURCE
    main.cxx
    ../Common/Common.h
    ../Common/Common.cxx
)
ADD_EXECUTABLE(
   ${FBX_TARGET_NAME}
   ${FBX_TARGET_SOURCE}
)

ADD_CUSTOM_COMMAND(
    TARGET ${FBX_TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FBX_ROOT}/samples/Audio/audio1.mp3" "${FBX_BUILD_ROOT}/audio1.mp3"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FBX_ROOT}/samples/Audio/audio2.wav" "${FBX_BUILD_ROOT}/audio2.wav"
)

SET_SAMPLES_GLOBAL_FLAGS()

