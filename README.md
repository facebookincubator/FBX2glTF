# FBX2GLTF

This is an interm fbx2gltf release.

Change skinning-weights to 4 if your engine does not support that feature.

Change the default import of the engine to be different from 30 fps if needed.

There are artifacts in the Github Actions for Windows, MacOS and Linux.

## For Linux

Use `./FBX2glTF-linux --pbr-metallic-roughness --skinning-weights 8 --anim-framerate bake30 --user-properties example.fbx`.

## For Windows

Use `FBX2glTF-windows.exe --pbr-metallic-roughness --skinning-weights 8 --anim-framerate bake30 --user-properties example.fbx`.

You need to install the MVSC redistributable. https://support.microsoft.com/en-ca/help/2977003/the-latest-supported-visual-c-downloads.

## For Mac

Use `./FBX2glTF-macos --pbr-metallic-roughness --skinning-weights 8 --anim-framerate bake30 --user-properties example.fbx`.

## Build Instructions

Reference the Github workflow.