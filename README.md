# FBX2glTF

This is a command line tool for converting 3D model assets on Autodesk's
venerable [FBX](https://www.autodesk.com/products/fbx/overview) format to
[glTF 2.0](https://github.com/KhronosGroup/glTF/tree/master/specification/2.0),
a modern runtime asset delivery format.

Precompiled binaries releases for Windows, Mac OS X and Linux may be
found [here](https://github.com/facebookincubator/FBX2glTF/releases).

## Running

The tool can be invoked like so:
```
 > FBX2glTF ~/models/butterfly.fbx
```

Or perhaps, as part of a more complex pipeline:
```
 > FBX2glTF --binary --draco --khr-materials-common \
          --input ~/models/source/butterfly.fbx \
          --output ~/models/target/butterfly.glb
```

### CLI Switches

You can always run the binary with --help to see what options it takes:
```
FBX2glTF 2.0: Generate a glTF 2.0 representation of an FBX model.
Usage:
  FBX2glTF [OPTION...] [<FBX File>]

  -i, --input arg               The FBX model to convert.
  -o, --output arg              Where to generate the output, without suffix.
  -e, --embed                   Inline buffers as data:// URIs within
                                generated non-binary glTF.
  -b, --binary                  Output a single binary format .glb file.
  -d, --draco                   Apply Draco mesh compression to geometries.
      --flip-u                  Flip all U texture coordinates.
      --flip-v                  Flip all V texture coordinates (default
                                behaviour!)
      --no-flip-v               Suppress the default flipping of V texture
                                coordinates
      --khr-materials-common    (WIP) Use KHR_materials_common extensions to
                                specify Unlit/Lambert/Blinn/Phong shaders.
      --pbr-metallic-roughness  (WIP) Try to glean glTF 2.0 native PBR
                                attributes from the FBX.
      --pbr-specular-glossiness
                                (WIP) Experimentally fill in the
                                KHR_materials_pbrSpecularGlossiness extension.
      --blend-shape-normals     Include blend shape normals, if reported
                                present by the FBX SDK.
      --blend-shape-tangents    Include blend shape tangents, if reported
                                present by the FBX SDK.
  -k, --keep-attribute arg      Used repeatedly to build a limiting set of
                                vertex attributes to keep.
  -v, --verbose                 Enable verbose output.
  -h, --help                    Show this help.
  -V, --version                 Display the current program version.
```

Some of these switches are not obvious:

- `--embed` is the way to get a single distributable file without using the
  binary format. It encodes the binary buffer(s) as a single enormous
  base64-encoded `data://` URI. This is a very slow and space-consuming way to
  accomplish what the binary format was invented to do simply and efficiently,
  but it can be useful e.g. for loaders that don't understand the .glb format.
- `--flip-u` and `--flip-v`, when enabled, will apply a `x -> (1.0 - x)`
  function to all `u` or `v` texture coordinates respectively. The `u` version
  is perhaps not commonly used, but flipping `v` is **the default behaviour*.
  Your FBX is likely constructed with the assumption that `(0, 0)` is bottom
  left, whereas glTF has `(0, 0)` as top left. To produce spec-compliant glTF,
  we must flip the texcoords. To request unflipped coordinates:
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

This build process has been tested on Linux, Mac OS X and Windows. It requires
CMake 3.5+ and a reasonably C++11 compliant toolchain.

We currently depend on the open source projects
[Draco](https://github.com/google/draco),
[MathFu](https://github.com/google/mathfu),
[Json](https://github.com/nlohmann/json),
[cppcodec](https://github.com/tplgy/cppcodec),
[cxxopts](https://github.com/jarro2783/cxxopts),
and [fmt](https://github.com/fmtlib/fmt);
all of which are automatically downloaded, configured and built.

You must manually download and install the
[Autodesk FBX SDK](https://www.autodesk.com/products/fbx/overview) and
accept its license agreement.

**At present, only version 2018.1.1 of the FBX SDK is supported**. The
build system will not successfully locate any other version.

### Linux and MacOS X
Compilation on Unix machines should be as simple as:

```
  > cd <FBX2glTF directory>
  > cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release
  > make -Cbuild -j4 install
```

If all goes well, you will end up with a statically linked executable.

### Windows

Windows users may [download](https://cmake.org/download) CMake for Windows,
install it and [run it](https://cmake.org/runningcmake/) on the FBX2glTF
checkout (choose a build directory distinct from the source).

As part of this process, you will be asked to choose which generator
to use. **At present, only Visual Studio 2017 is supported.** Older
versions of the IDE are unlikely to successfully build the tool.

*(MinGW support is plausible. The difficulty is linking statically against the
FBX SDK .lib file. Contributions welcome.)*

Note that the `CMAKE_BUILD_TYPE` variable from the Unix Makefile system is
entirely ignored here; it is when you open the generated solution that
you will be choose one of the canonical build types — *Debug*,
*Release*, *MinSizeRel*, and so on.

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

With glTF 2.0, we leaped headlong into physically-based rendering (BPR), where
the canonical way of expressing what a mesh looks like is by describing its
visible material in fundamental attributes like "how rough is this surface".

By contrast, FBX's material support remains in the older world of Lambert and
Phong, with simpler and more direct illumination and shading models. These modes
are largely incompatible — for example, textures in the old workflow often
contain baked lighting, which would arise naturally in a PBR environment.

Some material settings remain well supported and transfer automatically:
 - Emissive constants and textures
 - Occlusion maps
 - Normal maps

This leaves the other traditional settings of Lambert:
 - Ambient — this is anathema in the PBR world, where such effects should
   emerge naturally from the fundamental colour of the material and any ambient
   lighting present.
 - Diffuse — the material's direction-agnostic, non-specular reflection,
and additionally, with Blinn/Phong:
 - Specular — a more polished material's direction-sensitive reflection,
 - Shininess — just how polished the material is,

(All these can be either constants or textures.)

#### Exporting as Unlit/Lambert/Phong 
Increasingly with PBR materials, these properties are just left at zero or
default values in the FBX. But when they're there, and they're how you want the
glTF materials generated, one option is to use the --khr-materials-common
command line switch, with the awareness that this incurs a required dependency
on the glTF extension `KHR_materials_common`.

**Note that at the time of writing, this glTF extension is still undergoing the
ratification process, and is furthermore likely to change names.**

#### Exporting as Metallic-Roughness PBR
Given the command line flag --pbr-metallic-roughness, we accept glTF 2.0's PBR
mode, but we do so very partially, filling in a couple of reasonable constants
for metalness and roughness and using the diffuse texture, if it exists, as the
`base colour` texture.

More work is needed to harness the power of glTF's 2.0's materials. The biggest
issue here is the lack of any obviously emerging standards to complement FBX
itself. It's not clear what format an artist can export their PBR materials on,
and when they can, how to communicate this information well to `FBX2glTF`.

(*Stingray PBS* support is
[high on the TODO list](https://github.com/facebookincubator/FBX2glTF/issues/12).)

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
FBX2glTF is BSD-licensed. We also provide an additional patent grant.
