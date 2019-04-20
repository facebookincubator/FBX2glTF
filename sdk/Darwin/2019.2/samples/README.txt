================================================================================

                              FBX SDK SAMPLES README

Welcome to the FBX SDK samples readme! This document explains how to call CMake
to generate the appropriate build files.

Sincerely,
the Autodesk FBX team

================================================================================

To generate the build files you need to call CMake with the desired generator
and the following options (omitting an option will set it to it's default
value as shown below between [] or explicitly said:

FBX_SHARED     :  set at command line with -DFBX_SHARED=1 to enable the sample to 
                  link with the FBX SDK dynamic library
               
FBX_STATIC_RTL :  set at command line with -DFBX_STATIC_RTL=1 to use the static 
                  MSVCRT (/MT). By default will use the dynamic MSVCRT (/MD).
                  This option is only meaningful on the Windows platform.
                  
FBX_VARIANT    :  set at command line with -DFBX_VARIANT=[debug] or release.
                  This option is only meaningful on Unix/MacOS platforms.
                  On Windows, the generated solutions will contain the 
                  Debug, Release and RelWithDebInfo targets.

FBX_ARCH       :  set at command line with -DFBX_ARCH=[x64] or x86.
                  This option is only meaningful with the Make generator.
                  With the Visual Studio generators, this value is automatically
                  deduced based on the specified generator.


Although you can run cmake in the sample directory, to avoid mixing build files and source files
in the same folder, we strongly suggest you create a separate build folder and run cmake from there.

Examples:
==========

1) Building the ViewScene sample on Windows using Visual Studio 2015 (and the default settings):

    1. cd samples\ViewScene
    2. mkdir build
    3. cd build
    4. cmake -G "Visual Studio 14 Win64" ..

    5. Now that the ViewScene.sln and all the other related files have been generated in the samples\ViewScene\build,
       load the solution in Visual Studio and build it. The build result will be written in the bin directory at the root
       level of the FBX SDK installation.

If you want to re-generate the solution with different settings, it is preferable that you first delete the content of
the build folder to avoid cmake cache incompatibilities.

2) Building the ViewScene sample on Windows using Visual Studio 2015 32bits /MT

    cmake -G "Visual Studio 14" -DFBX_STATIC_RTL=1 ..
    
3) Building the ViewScene sample on Windows using Visual Studio 2015 64 bits DLL

    cmake -G "Visual Studio 14 Win64" -DFBX_SHARED=1 ..
    
4) Wrong configuration, will display a warning and generates a DLL version

    cmake -G "Visual Studio 14 Win64" -DFBX_SHARED=1 -DFBX_STATIC_RTL=1 ..
    Both FBX_SHARED and FBX_STATIC_RTL have been defined. They are mutually exclusive, considering FBX_SHARED only.

