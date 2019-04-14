import os

from conans import ConanFile, CMake

class FBX2glTFConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    requires = (("boost_filesystem/1.69.0@bincrafters/stable"),
               ("fmt/5.3.0@bincrafters/stable"))
#    generators = "cmake"
    generators = "cmake_find_package", "cmake_paths"

    def build(self):
        if os.environ.get('FBXSDK_SDKS') == None:
            print("Please set the environment variable FBXSDK_SDKS.")
            return
        cmake = CMake(self)
        cmake.definitions["FBXSDK_SDKS"] = os.getenv('FBXSDK_SDKS')
        cmake.configure()
        cmake.build()
