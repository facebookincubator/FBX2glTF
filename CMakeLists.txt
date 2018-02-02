cmake_minimum_required(VERSION 3.5)
project(FBX2glTF)

if ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR
    "Building from within the source tree is not supported.\n"
    "Hint: mkdir -p build; cmake -H. -Bbuild; make -Cbuild\n")
endif ()

set(CMAKE_CXX_STANDARD 11)
find_package(Threads REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
include(ExternalProject)

# FBX
find_package(FBX REQUIRED)
if (NOT FBXSDK_FOUND)
  message(FATAL_ERROR
    "Can't find FBX SDK in either:\n"
    " - Mac OS X: ${FBXSDK_APPLE_ROOT}\n"
    " - Windows: ${FBXSDK_WINDOWS_ROOT}\n"
    " - Linux: ${FBXSDK_LINUX_ROOT}"
)
endif()

# DRACO
ExternalProject_Add(Draco
  GIT_REPOSITORY https://github.com/google/draco
  GIT_TAG 1.2.5
  PREFIX draco
  INSTALL_DIR
  CMAKE_ARGS
	-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)
set(DRACO_INCLUDE_DIR "${CMAKE_BINARY_DIR}/draco/include")
if (WIN32)
   set(DRACO_LIB "${CMAKE_BINARY_DIR}/draco/lib/dracoenc.lib")
else()
   set(DRACO_LIB "${CMAKE_BINARY_DIR}/draco/lib/libdracoenc.a")
endif()

# MATHFU
set(mathfu_build_benchmarks OFF CACHE BOOL "")
set(mathfu_build_tests OFF CACHE BOOL "")
ExternalProject_Add(MathFu
  PREFIX mathfu
  GIT_REPOSITORY https://github.com/google/mathfu
  GIT_TAG v1.1.0
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo "Skipping MathFu configure step."
  BUILD_COMMAND ${CMAKE_COMMAND} -E echo "Skipping MathFu build step."
  INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping MathFu install step."
)
set(MATHFU_INCLUDE_DIRS
        "${CMAKE_BINARY_DIR}/mathfu/src/MathFu/include/"
        "${CMAKE_BINARY_DIR}/mathfu/src/MathFu/dependencies/vectorial/include")

# OrderedMap
ExternalProject_Add(FiFoMap
  PREFIX fifo_map
  GIT_REPOSITORY https://github.com/nlohmann/fifo_map
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo "Skipping FiFoMap configure step."
  BUILD_COMMAND ${CMAKE_COMMAND} -E echo "Skipping FiFoMap build step."
  INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping FiFoMap install step."
)
set(FIFO_MAP_INCLUDE_DIR "${CMAKE_BINARY_DIR}/fifo_map/src/FiFoMap/src")

# JSON
ExternalProject_Add(Json
  PREFIX json
  GIT_REPOSITORY https://github.com/nlohmann/json
  GIT_TAG v2.1.1
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo "Skipping JSON configure step."
  BUILD_COMMAND ${CMAKE_COMMAND} -E echo "Skipping JSON build step."
  INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping JSON install step."
)
set(JSON_INCLUDE_DIR "${CMAKE_BINARY_DIR}/json/src/Json/src")

# stb
ExternalProject_Add(STB
  PREFIX stb
  GIT_REPOSITORY https://github.com/nothings/stb
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo "Skipping STB configure step."
  BUILD_COMMAND ${CMAKE_COMMAND} -E echo "Skipping STB build step."
  INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping STB install step."
)
set(STB_INCLUDE_DIR "${CMAKE_BINARY_DIR}/stb/src/STB")

# cppcodec
ExternalProject_Add(CPPCodec
  PREFIX cppcodec
  GIT_REPOSITORY https://github.com/tplgy/cppcodec
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo "Skipping CPPCodec configure step."
  BUILD_COMMAND ${CMAKE_COMMAND} -E echo "Skipping CPPCodec build step."
  INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping CPPCodec install step."
)
set(CPPCODEC_INCLUDE_DIR "${CMAKE_BINARY_DIR}/cppcodec/src/CPPCodec")

# CXXOPTS
ExternalProject_Add(CxxOpts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts
  GIT_TAG v1.4.4
  PREFIX cxxopts
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo "Skipping cxxopts configure step."
  BUILD_COMMAND ${CMAKE_COMMAND} -E echo "Skipping cxxopts build step."
  INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping cxxopts install step."
)
set(CXXOPTS_INCLUDE_DIR "${CMAKE_BINARY_DIR}/cxxopts/src/CxxOpts/include")

# FMT
ExternalProject_Add(Fmt
  PREFIX fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG 4.0.0
  CMAKE_CACHE_ARGS  "-DFMT_DOC:BOOL=OFF" "-DFMT_INSTALL:BOOL=ON" "-DFMT_TEST:BOOL=OFF"
  CMAKE_ARGS
	-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)
set(FMT_INCLUDE_DIR "${CMAKE_BINARY_DIR}/fmt/include")
if (WIN32)
   set(FMT_LIB "${CMAKE_BINARY_DIR}/fmt/lib/fmt.lib")
else()
   set(FMT_LIB "${CMAKE_BINARY_DIR}/fmt/lib/libfmt.a")
endif()

if (APPLE)
    find_library(CF_FRAMEWORK CoreFoundation)
    message("CoreFoundation Framework: ${CF_FRAMEWORK}")
    set(FRAMEWORKS ${CF_FRAMEWORK})
endif()

set(SOURCE_FILES
        src/utils/File_Utils.cpp
        src/utils/Image_Utils.cpp
        src/utils/String_Utils.cpp
        src/main.cpp
        src/Fbx2Raw.cpp
        src/Raw2Gltf.cpp
        src/RawModel.cpp
        src/glTF/BufferData.cpp
        src/glTF/MaterialData.cpp
        src/glTF/MeshData.cpp
        src/glTF/NodeData.cpp
        src/glTF/PrimitiveData.cpp
        src/glTF/BufferViewData.cpp
        src/glTF/BufferViewData.h
        src/glTF/AccessorData.cpp
        src/glTF/AccessorData.h
        src/glTF/ImageData.cpp
        src/glTF/TextureData.cpp
        src/glTF/SkinData.cpp
        src/glTF/AnimationData.cpp
        src/glTF/CameraData.cpp
        src/glTF/SceneData.cpp
)

add_executable(FBX2glTF ${SOURCE_FILES})

add_dependencies(FBX2glTF
  Draco
  MathFu
  FiFoMap
  Json
  STB
  CxxOpts
  CPPCodec
  Fmt
)

if (NOT MSVC)
  # Disable annoying & spammy warning from FBX SDK header file
  target_compile_options(FBX2glTF PRIVATE
    "-Wno-null-dereference"
    "-Wunused"
    )
endif()

target_link_libraries(FBX2glTF
  ${FRAMEWORKS}
  ${CMAKE_DL_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
  ${DRACO_LIB}
  ${FMT_LIB}
  optimized ${FBXSDK_LIBRARY}
  debug ${FBXSDK_LIBRARY_DEBUG}
)

target_include_directories(FBX2glTF PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${FBXSDK_INCLUDE_DIR}
  ${DRACO_INCLUDE_DIR}
  ${MATHFU_INCLUDE_DIRS}
  ${FIFO_MAP_INCLUDE_DIR}
  ${JSON_INCLUDE_DIR}
  ${CXXOPTS_INCLUDE_DIR}
  ${STB_INCLUDE_DIR}
  ${CPPCODEC_INCLUDE_DIR}
  ${FMT_INCLUDE_DIR}
)

install (TARGETS FBX2glTF DESTINATION bin)
