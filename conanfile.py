# Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
#

import os

from conans import ConanFile, CMake


class FBX2glTFConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    requires = (("boost_filesystem/1.69.0@bincrafters/stable"),
                ("fmt/5.3.0@bincrafters/stable"))
    generators = "cmake_find_package", "cmake_paths"

    def build(self):
        cmake = CMake(self)
        cmake.definitions["FBXSDK_SDKS"] = os.getenv('FBXSDK_SDKS', 'sdk')
        cmake.configure()
        cmake.build()
