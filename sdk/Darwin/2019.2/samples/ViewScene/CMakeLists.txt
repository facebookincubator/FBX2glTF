SET(CMAKE_MIN_REQ_VERSION 2.4)
cmake_minimum_required(VERSION ${CMAKE_MIN_REQ_VERSION})
INCLUDE("../CMakeSettings.txt")

FBX_PROJECT(ViewScene .)
IF(CMAKE_OSX_SYSROOT MATCHES "Platforms/iPhone*")
    MESSAGE(FATAL_ERROR "on iOS we don't have OpenGL stuff so we cannot compile ViewScene")
    RETURN()
ENDIF()

SET(FBX_TARGET_SOURCE
    DrawScene.h
    GetPosition.h
    GlFunctions.h
    SetCamera.h
    SceneCache.h
    SceneContext.h
    DrawText.h
    targa.h
    DrawScene.cxx
    GetPosition.cxx
    GlFunctions.cxx
    SetCamera.cxx
    SceneCache.cxx
    SceneContext.cxx
    DrawText.cxx
    main.cxx
    targa.cxx
    ../Common/Common.h
    ../Common/Common.cxx
)

ADD_DEFINITIONS(
    ${FBX_BUILD_DEFINES}
    ${FBX_DEPENDENTS_DEFINES}
)

ADD_EXECUTABLE(
   ${FBX_TARGET_NAME}
   ${FBX_TARGET_SOURCE}
)

SET(SRC_LIB_ROOT ${FBX_ROOT}/samples/ViewScene/libs)
IF(WIN32)
    ADD_CUSTOM_COMMAND(
        TARGET ${FBX_TARGET_NAME} PRE_LINK
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SRC_LIB_ROOT}/win/glut${FBX_ARCH}/glew32.dll" "${FBX_BUILD_ROOT}/glew32.dll"
    )
    ADD_CUSTOM_COMMAND(
        TARGET ${FBX_TARGET_NAME} PRE_LINK
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SRC_LIB_ROOT}/win/glut${FBX_ARCH}/glut32.dll" "${FBX_BUILD_ROOT}/glut32.dll"
    )
    SET(FBX_REQUIRED_LIBS_DEPENDENCY "${SRC_LIB_ROOT}/win/glut${FBX_ARCH}/glew32.lib" ${FBX_REQUIRED_LIBS_DEPENDENCY})
    SET(FBX_REQUIRED_LIBS_DEPENDENCY "${SRC_LIB_ROOT}/win/glut${FBX_ARCH}/glut32.lib" ${FBX_REQUIRED_LIBS_DEPENDENCY})
ELSE(WIN32)
    IF (APPLE)
        SET(LIB_GLEW ${SRC_LIB_ROOT}/macos/libGLEW32.a)
        IF(FBX_ARCH STREQUAL "x64")
            SET(LIB_GLEW ${SRC_LIB_ROOT}/macos/libGLEW64.a)
        ENDIF()
        SET(FBX_LINKER_FLAGS "-framework GLUT -framework OpenGL -framework Cocoa")
        SET(FBX_REQUIRED_LIBS_DEPENDENCY ${LIB_GLEW} ${FBX_REQUIRED_LIBS_DEPENDENCY})
    ELSEIF(LINUX)
        IF(FBX_ARCH STREQUAL "x64")
            SET(LIB_GLEW ${SRC_LIB_ROOT}/linux/libGLEW_amd64.a)
            SET(LIB_GLUT ${SRC_LIB_ROOT}/linux/libglut_gcc34-amd64.a)
        ELSE()
            SET(LIB_GLEW ${SRC_LIB_ROOT}/linux/libGLEW.a)
            SET(LIB_GLUT ${SRC_LIB_ROOT}/linux/libglut_gcc32.a)
        ENDIF()
        SET(FBX_REQUIRED_LIBS_DEPENDENCY ${LIB_GLEW} ${FBX_REQUIRED_LIBS_DEPENDENCY})
        SET(FBX_REQUIRED_LIBS_DEPENDENCY ${LIB_GLUT} ${FBX_REQUIRED_LIBS_DEPENDENCY})
        SET(FBX_REQUIRED_LIBS_DEPENDENCY ${FBX_REQUIRED_LIBS_DEPENDENCY} GLU GL X11)
    ENDIF(APPLE)
ENDIF(WIN32)

SET_SAMPLES_GLOBAL_FLAGS()

ADD_CUSTOM_COMMAND(
    TARGET ${FBX_TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FBX_ROOT}/samples/ViewScene/humanoid.fbx" "${FBX_BUILD_ROOT}/humanoid.fbx"
    
    TARGET ${FBX_TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FBX_ROOT}/samples/ViewScene/FBX_ASCII_TEXTURE" "${FBX_BUILD_ROOT}/FBX_ASCII_TEXTURE"
)
