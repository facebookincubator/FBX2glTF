cmake_minimum_required(VERSION 3.5)
project(FBX2glTF)

set(typical_usage_str
  "Example usage:\n\
  > mkdir -p build_debug\n\
  > conan install . -i build_debug -s build_type=Debug -e FBXSDK_SDKS=/home/zell/FBXSDK\n\
  > conan build . -bf build_debug")

if ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR
    "Building from within the source tree is not supported! ${typical_usage_str}")
endif ()

set(CMAKE_CXX_STANDARD 11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
include(ExternalProject)

# FBX
foreach (FBXSDK_VERSION "2020.2")
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

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan_paths.cmake")
  message(FATAL_ERROR
    "The Conan package manager must run ('install') first. ${typical_usage_str}")
endif()
include("${CMAKE_BINARY_DIR}/conan_paths.cmake")

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_BINARY_DIR}")

# stuff we get from Conan
find_package(Boost 1.76 COMPONENTS system filesystem program_options REQUIRED )
find_package(LibXml2 MODULE REQUIRED)
find_package(ZLIB MODULE REQUIRED)
find_package(fmt MODULE REQUIRED)

# create a compilation database for e.g. clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# DRACO
ExternalProject_Add(Draco
  GIT_REPOSITORY https://github.com/google/draco
  GIT_TAG 75b82f7b68c797b4e0ff5e73ca8a81e2db3db797
  PREFIX draco
  INSTALL_DIR
  CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DBUILD_FOR_GLTF=1
)
set(DRACO_INCLUDE_DIR "${CMAKE_BINARY_DIR}/draco/include")
find_file(FEDORA_FOUND fedora-release
  PATHS /etc
)
if (WIN32)
   set(DRACO_LIB "${CMAKE_BINARY_DIR}/draco/lib/${CMAKE_SHARED_MODULE_PREFIX}draco.lib")
elseif(FEDORA_FOUND)
  set(DRACO_LIB "${CMAKE_BINARY_DIR}/draco/lib64/${CMAKE_SHARED_MODULE_PREFIX}draco.a")
else()
  set(DRACO_LIB "${CMAKE_BINARY_DIR}/draco/lib/${CMAKE_SHARED_MODULE_PREFIX}draco.a")
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


# cppcodec
ExternalProject_Add(CPPCodec
  PREFIX cppcodec
  GIT_REPOSITORY https://github.com/tplgy/cppcodec
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo "Skipping CPPCodec configure step."
  BUILD_COMMAND ${CMAKE_COMMAND} -E echo "Skipping CPPCodec build step."
  INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping CPPCodec install step."
)
set(CPPCODEC_INCLUDE_DIR "${CMAKE_BINARY_DIR}/cppcodec/src/CPPCodec")

if (APPLE)
    find_library(CF_FRAMEWORK CoreFoundation)
    message("CoreFoundation Framework: ${CF_FRAMEWORK}")
    set(FRAMEWORKS ${CF_FRAMEWORK})
endif()

set(LIB_SOURCE_FILES
        src/FBX2glTF.h
        src/fbx/materials/ArnoldStandardMaterial.cpp
        src/fbx/materials/3dsMaxPhysicalMaterial.cpp
        src/fbx/materials/FbxMaterials.cpp
        src/fbx/materials/FbxMaterials.hpp
        src/fbx/materials/RoughnessMetallicMaterials.hpp
        src/fbx/materials/StingrayPBSMaterial.cpp
        src/fbx/materials/TraditionalMaterials.cpp
        src/fbx/materials/TraditionalMaterials.hpp
        src/fbx/Fbx2Raw.cpp
        src/fbx/Fbx2Raw.hpp
        src/fbx/FbxBlendShapesAccess.cpp
        src/fbx/FbxBlendShapesAccess.hpp
        src/fbx/FbxLayerElementAccess.hpp
        src/fbx/FbxSkinningAccess.cpp
        src/fbx/FbxSkinningAccess.hpp
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
        src/gltf/properties/LightData.cpp
        src/gltf/properties/LightData.hpp
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
        src/utils/String_Utils.hpp
        third_party/CLI11/CLI11.hpp
)

add_library(libFBX2glTF STATIC ${LIB_SOURCE_FILES})
set_target_properties(libFBX2glTF PROPERTIES OUTPUT_NAME "FBX2glTF")
add_executable(appFBX2glTF src/FBX2glTF.cpp)
set_target_properties(appFBX2glTF PROPERTIES OUTPUT_NAME "FBX2glTF")

add_dependencies(libFBX2glTF
  Draco
  MathFu
  FiFoMap
  CPPCodec
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
  Boost::system
  Boost::filesystem
  optimized ${FBXSDK_LIBRARY}
  debug ${FBXSDK_LIBRARY_DEBUG}
  fmt::fmt
  xml2::xml2
  ZLIB::ZLIB
  ${CMAKE_DL_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
)

find_package(Iconv REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Iconv::Iconv)

target_include_directories(libFBX2glTF PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_include_directories(libFBX2glTF SYSTEM PUBLIC
  "third_party/stb"
  "third_party/json"
  ${FBXSDK_INCLUDE_DIR}
  ${DRACO_INCLUDE_DIR}
  ${MATHFU_INCLUDE_DIRS}
  ${FIFO_MAP_INCLUDE_DIR}
  ${CPPCODEC_INCLUDE_DIR}
)

target_include_directories(appFBX2glTF PUBLIC
  "third_party/CLI11"
)
target_link_libraries(appFBX2glTF libFBX2glTF)

install (TARGETS libFBX2glTF appFBX2glTF
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
)
