# FBX2glTF

[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)


This is a command line tool for converting 3D model assets on the
well-established [FBX](https://www.autodesk.com/products/fbx/overview) format to
[glTF 2.0](https://github.com/KhronosGroup/glTF/tree/master/specification/2.0),
a modern runtime asset delivery format.

# Platform Binaries

This package contains three versions of `FBX2glTF`, compiled for three platforms
and located in three eponymous directories:
 - bin/Darwin/FBX2glTF
 - bin/Linux/FBX2glTF
 - bin/Windows_NT/FBX2glTF.exe

# Usage

```js
/**
 * Converts an FBX to a GTLF or GLB file.
 * @param string srcFile path to the source file.
 * @param string destFile path to the destination file.
 * This must end in `.glb` or `.gltf` (case matters).
 * @param string[] [opts] options to pass to the converter tool.
 * @return Promise<string> a promise that yields the full path to the converted
 * file, an error on conversion failure.
 */
convert(srcPath :string, destPath :string, args :?string[]) :Promise<string>
```

For example:

```js
const convert = require('fbx2gltf');
convert('path/to/some.fbx', 'path/to/target.glb', ['--khr-materials-unlit']).then(
  destPath => {
    // yay, do what we will with our shiny new GLB file!
  },
  error => {
    // ack, conversion failed: inspect 'error' for details
  }
);
```

# Further Reading

The home of this tool is [here](https://github.com/facebookincubator/FBX2glTF).

# Authors
 - Pär Winzell
 - J.M.P. van Waveren
 - Amanda Watson

# Legal

FBX2glTF is licensed under the [3-clause BSD license](LICENSE).

```
This software contains Autodesk® FBX® code developed by Autodesk, Inc. Copyright
2017 Autodesk, Inc. All rights, reserved. Such code is provided “as is” and
Autodesk, Inc. disclaims any and all warranties, whether express or implied,
including without limitation the implied warranties of merchantability, fitness
for a particular purpose or non-infringement of third party rights. In no event
shall Autodesk, Inc. be liable for any direct, indirect, incidental, special,
exemplary, or consequential damages (including, but not limited to, procurement
of substitute goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability, whether in
contract, strict liability, or tort (including negligence or otherwise) arising
in any way out of such code.
```
