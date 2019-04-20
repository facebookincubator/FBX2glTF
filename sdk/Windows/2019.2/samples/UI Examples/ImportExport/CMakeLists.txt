SET(CMAKE_MIN_REQ_VERSION 2.4)
cmake_minimum_required(VERSION ${CMAKE_MIN_REQ_VERSION})
INCLUDE("../CMakeSettings.txt")
IF(NOT WIN32)
    MESSAGE(SEND_ERROR "This sample can only compile on Windows platform.")
    RETURN()
ENDIF()

FBX_PROJECT(UI_Example_ImportExport .)
SET(FBX_TARGET_SOURCE
    "../Common/ImportExport.cxx"
    "../Common/ImportExport.h"
    "../SceneTreeView/UI.ico"
    "../SceneTreeView/UI.rc"
    "resource.h"
    "stdafx.h"
    "UI.cxx"
    "UI.h"
)
ADD_EXECUTABLE(
    ${FBX_TARGET_NAME}
    WIN32    #sets the application subsystem to Windows Application instead of Console
    ${FBX_TARGET_SOURCE}
)
SET_SAMPLES_GLOBAL_FLAGS()
