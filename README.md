# FBX2GLTF

This is an interm fbx2gltf release.

Change skinning-weights to 4 if your engine does not support that feature.

Change the default import of the engine to be different from 30 fps if needed.

## For Linux

Use `./FBX2glTF-linux-64 --pbr-metallic-roughness --skinning-weights 8 --anim-framerate bake30 --user-properties example.fbx`.

## For Windows

Use `FBX2glTF-windows-64.exe --pbr-metallic-roughness --skinning-weights 8 --anim-framerate bake30 --user-properties example.fbx`.

You need to install the MVSC redistributable. https://support.microsoft.com/en-ca/help/2977003/the-latest-supported-visual-c-downloads.

## For Mac

Use `./FBX2glTF-macosx --pbr-metallic-roughness --skinning-weights 8 --anim-framerate bake30 --user-properties example.fbx`.

## Build Instructions

```
# Determine SDK location & build settings for Linux vs (Recent) Mac OS X
> if [[ "$OSTYPE" == "darwin" ]]; then
    export CONAN_CONFIG="-s compiler=apple-clang -s compiler.version=11.0 -s compiler.libcxx=libc++"
    export FBXSDK_TARBALL="https://github.com/V-Sekai/FBXSDK-Darwin/archive/2019.2.tar.gz"
else
    export CONAN_CONFIG="-s compiler.libcxx=libstdc++11"
    export FBXSDK_TARBALL="https://github.com/V-Sekai/FBXSDK-Linux/archive/2020.2.tar.gz"
  fi

# Fetch Project
> GIT_LFS_SKIP_SMUDGE=1 git clone https://github.com/V-Sekai/FBX2glTF.git
> cd FBX2glTF

# Fetch and unpack FBX SDK
> curl -sL "${FBXSDK_TARBALL}" | tar xz --strip-components=1 --wildcards */sdk
# Fetch and unpack FBX SDK on Mac OS X
> curl -sL "${FBXSDK_TARBALL}" | tar xz --strip-components=1 
# Then decompress the contents
> zstd -d -r --rm sdk

# Install and configure Conan, if needed
> pip3 install conan --user # or sometimes just "pip"; you may need to install Python/PIP
> conan remote add --force bincrafters https://api.bintray.com/conan/bincrafters/public-conan

# Initialize & run build
> conan install . -i build -s build_type=Release ${CONAN_CONFIG} --build fmt --build boost_system --build boost_filesystem --build libiconv --build=libxml2 --build=zlib
> conan build . -bf build
```
