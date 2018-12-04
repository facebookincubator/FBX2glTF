cmake_minimum_required(VERSION 3.5)
project(FBX2glTF)

if ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR
    "Building from within the source tree is not supported.\n"
    "Hint: mkdir -p build; cmake -H. -Bbuild; make -Cbuild\n")
endif ()

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(ZLIB REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
include(ExternalProject)

# FBX
foreach (FBXSDK_VERSION "2019.2")
  find_package(FBX)
  if (FBXSDK_FOUND)
    break()
  endif()
endforeach(FBXSDK_VERSION)
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
  GIT_TAG 1.3.4
  PREFIX draco
  INSTALL_DIR
  CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DBUILD_FOR_GLTF=1
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

set(LIB_SOURCE_FILES
        src/FBX2glTF.h
        src/fbx/Fbx2Raw.cpp
        src/fbx/Fbx2Raw.hpp
        src/fbx/FbxBlendShapesAccess.cpp
        src/fbx/FbxBlendShapesAccess.hpp
        src/fbx/FbxLayerElementAccess.hpp
        src/fbx/FbxMaterialInfo.hpp
        src/fbx/FbxMaterialsAccess.cpp
        src/fbx/FbxMaterialsAccess.hpp
        src/fbx/FbxRoughMetMaterialInfo.cpp
        src/fbx/FbxRoughMetMaterialInfo.hpp
        src/fbx/FbxSkinningAccess.cpp
        src/fbx/FbxSkinningAccess.hpp
        src/fbx/FbxTraditionalMaterialInfo.cpp
        src/fbx/FbxTraditionalMaterialInfo.hpp
        src/gltf/Raw2Gltf.cpp
        src/gltf/Raw2Gltf.hpp
        src/gltf/GltfModel.cpp
        src/gltf/GltfModel.hpp
        src/gltf/TextureBuilder.cpp
        src/gltf/TextureBuilder.hpp
        src/gltf/properties/AccessorData.cpp
        src/gltf/properties/AccessorData.hpp
        src/gltf/properties/AnimationData.cpp
        src/gltf/properties/AnimationData.hpp
        src/gltf/properties/BufferData.cpp
        src/gltf/properties/BufferData.hpp
        src/gltf/properties/BufferViewData.cpp
        src/gltf/properties/BufferViewData.hpp
        src/gltf/properties/CameraData.cpp
        src/gltf/properties/CameraData.hpp
        src/gltf/properties/ImageData.cpp
        src/gltf/properties/ImageData.hpp
        src/gltf/properties/MaterialData.cpp
        src/gltf/properties/MaterialData.hpp
        src/gltf/properties/MeshData.cpp
        src/gltf/properties/MeshData.hpp
        src/gltf/properties/NodeData.cpp
        src/gltf/properties/NodeData.hpp
        src/gltf/properties/PrimitiveData.cpp
        src/gltf/properties/PrimitiveData.hpp
        src/gltf/properties/SamplerData.hpp
        src/gltf/properties/SceneData.cpp
        src/gltf/properties/SceneData.hpp
        src/gltf/properties/SkinData.cpp
        src/gltf/properties/SkinData.hpp
        src/gltf/properties/TextureData.cpp
        src/gltf/properties/TextureData.hpp
        src/mathfu.hpp
        src/raw/RawModel.cpp
        src/raw/RawModel.hpp
        src/utils/File_Utils.cpp
        src/utils/File_Utils.hpp
        src/utils/Image_Utils.cpp
        src/utils/Image_Utils.hpp
        src/utils/String_Utils.cpp
        src/utils/String_Utils.hpp
)

add_library(libFBX2glTF STATIC ${LIB_SOURCE_FILES})
set_target_properties(libFBX2glTF PROPERTIES OUTPUT_NAME "FBX2glTF")
add_executable(appFBX2glTF src/FBX2glTF.cpp)
set_target_properties(appFBX2glTF PROPERTIES OUTPUT_NAME "FBX2glTF")

add_dependencies(libFBX2glTF
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
  target_compile_options(libFBX2glTF PRIVATE
    "-Wno-null-dereference"
    "-Wunused"
    )
  target_compile_options(appFBX2glTF PRIVATE
    "-Wno-null-dereference"
    "-Wunused"
    )
endif()

target_link_libraries(libFBX2glTF
  ${FRAMEWORKS}
  ${DRACO_LIB}
  ${FMT_LIB}
  optimized ${FBXSDK_LIBRARY}
  debug ${FBXSDK_LIBRARY_DEBUG}
  ${CMAKE_DL_LIBS}
  ${LIBXML2_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
)

target_include_directories(libFBX2glTF PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_include_directories(libFBX2glTF SYSTEM PUBLIC
  ${FBXSDK_INCLUDE_DIR}
  ${DRACO_INCLUDE_DIR}
  ${MATHFU_INCLUDE_DIRS}
  ${FIFO_MAP_INCLUDE_DIR}
  ${JSON_INCLUDE_DIR}
  ${STB_INCLUDE_DIR}
  ${CPPCODEC_INCLUDE_DIR}
  ${FMT_INCLUDE_DIR}
)

target_include_directories(appFBX2glTF PUBLIC
  ${CXXOPTS_INCLUDE_DIR}
)
target_link_libraries(appFBX2glTF libFBX2glTF)

install (TARGETS libFBX2glTF appFBX2glTF
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
)
