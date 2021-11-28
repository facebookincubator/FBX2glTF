# Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
#

import os

from conans import ConanFile, CMake


class FBX2glTFConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    requires = (
        ("boost/1.76.0"),
        ("libiconv/1.15"),
        ("zlib/1.2.11"),
        ("libxml2/2.9.12"),
        ("fmt/5.3.0"),
    )
    generators = "cmake_find_package", "cmake_paths"

    def configure(self):
        pass

    def build(self):
        cmake = CMake(self)
        cmake.definitions["FBXSDK_SDKS"] = os.getenv("FBXSDK_SDKS", "sdk")
        cmake.configure()
        cmake.build()
