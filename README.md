# FBX2glTF
[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

This is a command line tool for converting 3D model assets on Autodesk's
venerable [FBX](https://www.autodesk.com/products/fbx/overview) format to
[glTF 2.0](https://github.com/KhronosGroup/glTF/tree/master/specification/2.0),
a modern runtime asset delivery format.

Precompiled binaries releases for Windows, Mac OS X and Linux may be
found [here](https://github.com/facebookincubator/FBX2glTF/releases).

Bleeding-edge binaries for Windows may be found [here](https://ci.appveyor.com/project/Facebook/fbx2gltf/build/artifacts). Linux and Mac OS X to come; meanwhile, you can [build your own](#building-it-on-your-own).

[![Build Status](https://travis-ci.com/facebookincubator/FBX2glTF.svg?branch=master)](https://travis-ci.com/facebookincubator/FBX2glTF)
[![Build status](https://ci.appveyor.com/api/projects/status/5mq4vbc44vmyec4w?svg=true)](https://ci.appveyor.com/project/Facebook/fbx2gltf)

## Running

The tool can be invoked like so:
```
 > FBX2glTF ~/models/butterfly.fbx
```

Or perhaps, as part of a more complex pipeline:
```
 > FBX2glTF --binary --draco --verbose \
          --input ~/models/source/butterfly.fbx \
          --output ~/models/target/butterfly.glb
```

There are also some friendly & hands-on instructions available [over at Facebook](https://developers.facebook.com/docs/sharing/3d-posts/glb-tutorials/#convert-from-fbx).

### CLI Switches

You can always run the binary with --help to see what options it takes:
```
FBX2glTF 0.9.6: Generate a glTF 2.0 representation of an FBX model.
Usage: FBX2glTF [OPTIONS] [FBX Model]

Positionals:
  FBX Model FILE              The FBX model to convert.

Options:
  -h,--help                   Print this help message and exit
  -v,--verbose                Include blend shape tangents, if reported present by the FBX SDK.
  -V,--version
  -i,--input FILE             The FBX model to convert.
  -o,--output TEXT            Where to generate the output, without suffix.
  -e,--embed                  Inline buffers as data:// URIs within generated non-binary glTF.
  -b,--binary                 Output a single binary format .glb file.
  --long-indices (never|auto|always)
                              Whether to use 32-bit indices.
  --compute-normals (never|broken|missing|always)
                              When to compute vertex normals from mesh geometry.
  --anim-framerate (bake24|bake30|bake60)
                              Select baked animation framerate.
  --flip-u                    Flip all U texture coordinates.
  --no-flip-u                 Don't flip U texture coordinates.
  --flip-v                    Flip all V texture coordinates.
  --no-flip-v                 Don't flip V texture coordinates.
  --no-khr-lights-punctual    Don't use KHR_lights_punctual extension to export FBX lights.
  --user-properties           Transcribe FBX User Properties into glTF node and material 'extras'.
  --blend-shape-normals       Include blend shape normals, if reported present by the FBX SDK.
  --blend-shape-tangents      Include blend shape tangents, if reported present by the FBX SDK.
  -k,--keep-attribute (position|normal|tangent|binormial|color|uv0|uv1|auto) ...
                              Used repeatedly to build a limiting set of vertex attributes to keep.


Materials:
  --pbr-metallic-roughness    Try to glean glTF 2.0 native PBR attributes from the FBX.
  --khr-materials-unlit       Use KHR_materials_unlit extension to request an unlit shader.


Draco:
  -d,--draco                  Apply Draco mesh compression to geometries.
  --draco-compression-level INT in [0 - 10]=7
                              The compression level to tune Draco to.
  --draco-bits-for-position INT in [1 - 32]=14
                              How many bits to quantize position to.
  --draco-bits-for-uv INT in [1 - 32]=10
                              How many bits to quantize UV coordinates to.
  --draco-bits-for-normals INT in [1 - 32]=10
                              How many bits to quantize nornals to.
  --draco-bits-for-colors INT in [1 - 32]=8
                              How many bits to quantize colors to.
  --draco-bits-for-other INT in [1 - 32]=8
                              How many bits to quantize all other vertex attributes to.
```

Some of these switches are not obvious:

- `--embed` is the way to get a single distributable file without using the
  binary format. It encodes the binary buffer(s) as a single base64-encoded
  `data://` URI. This is a very slow and space-consuming way to accomplish what
  the binary format was invented to do simply and efficiently, but it can be
  useful e.g. for loaders that don't understand the .glb format.
- `--flip-u` and `--flip-v`, when enabled, will apply a `x -> (1.0 - x)`
  function to all `u` or `v` texture coordinates respectively. The `u` version
  is perhaps not commonly used, but flipping `v` is **the default behaviour**.
  Your FBX is likely constructed with the assumption that `(0, 0)` is bottom
  left, whereas glTF has `(0, 0)` as top left. To produce spec-compliant glTF,
  we must flip the texcoords. To request unflipped coordinates:
- `--long-indices` lets you force the use of either 16-bit or 32-bit indices.
  The default option is auto, which make the choice on a per-mesh-size basis.
- `--compute-normals` controls when automatic vertex normals should be computed
  from the mesh. By default, empty normals (which are forbidden by glTF) are
  replaced. A choice of 'missing' implies 'broken', but additionally creates
  normals for models that lack them completely. 
- `--no-flip-v` will actively disable v coordinat flipping. This can be useful
  if your textures are pre-flipped, or if for some other reason you were already
  in a glTF-centric texture coordinate system.
- All three material options are, in their own way, works in progress, but the
  `--pbr-metallic-roughness` switch is at least compliant with the core spec;
  unlike the others, it does not depend on an unratified extension. That option
  will be chosen by default if you supply none of the others. Material switches
  are documented further below.
- If you supply any `-keep-attribute` option, you enable a mode wherein you must
  supply it repeatedly to list *all* the vertex attributes you wish to keep in
  the conversion process. This is a way to trim the size of the resulting glTF
  if you know the FBX contains superfluous attributes. The supported arguments
  are `position`, `normal`, `tangent`, `color`, `uv0`, and `uv1`.
- When **blend shapes** are present, you may use `--blend-shape-normals` and
  `--blend-shape-tangents` to include normal and tangent attributes in the glTF
  morph targets. They are not included by default because they rarely or never
  seem to be correctly present in the actual FBX source, which means the SDK
  must be computing them from geometry, unasked? In any case, they are beyond
  the control of the artist, and can yield strange crinkly behaviour. Since
  they also take up significant space in the output file, we made them opt-in.

## Building it on your own

We currently depend on the open source projects
[Draco](https://github.com/google/draco),
[MathFu](https://github.com/google/mathfu),
[Json](https://github.com/nlohmann/json),
[cppcodec](https://github.com/tplgy/cppcodec),
[CLI11](https://github.com/CLIUtils/CLI11),
[stb](https://github.com/nothings/stb),
and [fmt](https://github.com/fmtlib/fmt);
all of which are automatically downloaded and/or built.

**At present, only version 2019.2 of the FBX SDK is supported**. The
build system will not successfully locate any other version.

### Linux and MacOS X
Your development environment will need to have:
- build essentials (gcc for Linux, clang for Mac)
- cmake
- python 3.* and associated pip3/pip command
- zstd

Then, compilation on Unix machines will look something like:

```
# Determine SDK location & build settings for Linux vs (Recent) Mac OS X
> if [[ "$OSTYPE" == "darwin" ]]; then
    export CONAN_CONFIG="-s compiler=apple-clang -s compiler.version=10.0 -s compiler.libcxx=libc++"
    export FBXSDK_TARBALL="https://github.com/zellski/FBXSDK-Darwin/archive/2019.2.tar.gz"
else
    export CONAN_CONFIG="-s compiler.libcxx=libstdc++11"
    export FBXSDK_TARBALL="https://github.com/zellski/FBXSDK-Linux/archive/2019.2.tar.gz"
  fi

# Fetch Project
> GIT_LFS_SKIP_SMUDGE=1 git clone https://github.com/facebookincubator/FBX2glTF.git
> cd FBX2glTF

# Fetch and unpack FBX SDK
> curl -sL "${FBXSDK_TARBALL}" | tar xz --strip-components=1 --wildcards */sdk
# Then decompress the contents
> zstd -d -r --rm sdk

# Install and configure Conan, if needed
> pip3 install conan # or sometimes just "pip"; you may need to install Python/PIP
> conan remote add --force bincrafters https://api.bintray.com/conan/bincrafters/public-conan

# Initialize & run build
> conan install . -i build -s build_type=Release ${CONAN_CONFIG}
> conan build . -bf build
```

If all goes well, you will end up with a statically linked executable in `./build/FBX2glTF`.

### Windows

#### Install FBX SDK

2019.2 version is required.

#### Let FBX2glTF know where to find your FBX SDK

By default, FBX2glTF will look into `<FBX2glTF-root-dir>/sdk/<your-platform>/<FBX-SDK-version>`.
Usually you didn't install the FBX SDK into that directory.
So simply create a symbolic link to the real FBX SDK here.

The following scripts is powershell scripts.

First you should create the `sdk` directory if it doesn't exist. cd it after all.

```ps1
> mkdir sdk
> cd sdk
```

Now create the symbolic link. (You possibly need the admin privilege)

```ps1
> new-item -Path Windows -itemtype SymbolicLink -Value "path-to-fbx-sdk"
```

the `path-to-fbx-sdk` should be something like "C:\Program Files\Autodesk\FBX\FBX SDK".

#### Install dependencies via [vcpkg](https://github.com/microsoft/vcpkg)

```ps1
> vcpkg install boost-filesystem:x64-windows boost-optional:x64-windows libxml2:x64-windows zlib:x64-windows fmt:x64-windows
```

#### Tell FBX2glTF you are going to use vcpkg instead of connan

Add `-DFBX2GLTF_USE_VCPKG=ON` to cmake command line arguments.

## Conversion Process
The actual translation begins with the FBX SDK parsing the input file, and ends
with the generation of the descriptive `JSON` that forms the core of glTF, along
with binary buffers that hold geometry and animations (and optionally also
emedded resources such as textures.)

In the process, each mesh is ripped apart into a long list of triangles and
their associated vertices, with a material assigned to each one. A similar
process happens in reverse when we construct meshes and materials that conform
to the expectations of the glTF format.

### Animations
Every animation in the FBX file becomes an animation in the glTF file. The
method used is one of "baking": we step through the interval of time spanned by
the animation, keyframe by keyframe, calculate the local transform of each
node, and whenever we find any node that's rotated, translated or scaled, we
record that fact in the output.

Beyond skeleton-based animation, *Blend Shapes* are also supported; they are
read from the FBX file on a per-mesh basis, and clips can use them by varying
the weights associated with each one.

The baking method has the benefit of being simple and precise. It has the
drawback of creating potentially very large files. The more complex the
animation rig, the less avoidable this data explosion is.

There are three future enhancements we hope to see for animations:
- Version 2.0 of glTF brought us support for expressing quadratic animation
  curves, where previously we had only had linear. Not coincidentally, quadratic
  splines are one of the key ways animations are expressed inside the FBX. When
  we find such a curve, it would be more efficient to output it without baking
  it into a long sequence of linear approximations.
- We do not yet ever generate
  [sparse accessors](https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#sparse-accessors),
  but many animations (especially morph targets) would benefit from this
  storage optimisation.
- Perhaps most useful in practice is the idea of compressing animation curves
  the same way we use Draco to compress meshes (see below). Like geometry,
  animations are highly redundant — each new value is highly predictable from
  preceding values. If Draco extends its support for animations (it's on their
  roadmap), or if someone else develops a glTF extension for animation
  compression, we will likely add support in this tool.

### Materials

With glTF 2.0, we leaped headlong into physically-based rendering (PBR), where
the canonical way of expressing what a mesh looks like is by describing its
visible material in fundamental attributes like "how rough is this surface".

By contrast, FBX's material support remains largely in the older world of
Lambert and Phong, with simpler and more direct illumination and shading
models. These modes are inherently incompatible — for example, textures in the
old workflow often contain baked lighting of the type that would arise naturally
in a PBR environment.

Some material settings remain well supported and transfer automatically:
 - Emissive constants and textures
 - Occlusion maps
 - Normal maps

This leaves the other traditional settings, first of Lambert:
 - Ambient — this is anathema in the PBR world, where such effects should
   emerge naturally from the fundamental colour of the material and any ambient
   lighting present.
 - Diffuse — the material's direction-agnostic, non-specular reflection,
and additionally, with Blinn/Phong:
 - Specular — a more polished material's direction-sensitive reflection,
 - Shininess — just how polished the material is; a higher value here yields a
   more mirror-like surface.

(All these can be either constants or textures.)

#### Exporting as Unlit
If you have a model was constructed using an unlit workflow, e.g. a photogrammetry
capture or a landscape with careful baked-in lighting, you may choose to export
it using the --khr-materials-common switch. This incurs a dependency on the glTF
extension 'KHR_materials_unlit; a client that accepts  that extension is making
a promise it'll do its best to render pixel values without lighting calculations.

**Note that at the time of writing, this glTF extension is still undergoing the
ratification process**

#### Exporting as Metallic-Roughness PBR
Given the command line flag --pbr-metallic-roughness, we throw ourselves into
the warm embrace of glTF 2.0's PBR preference.

As mentioned above, there is little consensus in the world on how PBR should be
represented in FBX. At present, we support only one format: Stingray PBS. This
is a feature that comes bundled with Maya, and any PBR model exported through
that route should be digested propertly by FBX2glTF.

(A happy note: Allegorithmic's Substance Painter also exports Stingray PBS,
when hooked up to Maya.)

## Draco Compression
The tool will optionally apply [Draco](https://github.com/google/draco)
compression to the geometric data of each mesh (vertex indices, positions,
normals, per-vertex color, and so on). This can be dramatically effective
in reducing the size of the output file, especially for static models.

Enabling this feature adds an expressed required dependency in the glTF on the
`KHR_draco_geometry_compression` extension, and can thus only be loaded by a
viewer that is willing and able to decompress the data.

**Note that at the time of writing, this glTF extension is still undergoing the
ratification process.**

## Future Improvements
This tool is under continuous development. We do not have a development roadmap
per se, but some aspirations have been noted above. The canonical list of active
TODO items can be found
[on GitHub](https://github.com/facebookincubator/FBX2glTF/labels/enhancement).


## Authors
 - Pär Winzell
 - J.M.P. van Waveren
 - Amanda Watson

## License
FBX2glTF is licensed under the [3-clause BSD license](LICENSE).
