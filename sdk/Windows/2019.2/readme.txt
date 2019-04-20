================================================================================

                                     README

                             Autodesk FBX SDK 2019
                             -----------------------


Welcome to the FBX SDK readme! This document includes the latest changes since
the previous release version of the Autodesk FBX SDK, such as new features, bug
fixes, known issues, deprecated functions/classes and previous releases notes.

For more information, please visit us at http://www.autodesk.com/fbx/

To join the FBX Beta Program, please visit the Autodesk Feedback Community site
at http://beta.autodesk.com

Sincerely,
the Autodesk FBX team

================================================================================



TABLE OF CONTENTS
-----------------

    1. Features, Improvements And Changes
    2. Fixed And Known Issues
    3. Release Notes From Previous Releases
    4. Legal Disclaimer 



1. FEATURES, IMPROVEMENTS AND CHANGES
-------------------------------------

1.1 Improvements and New Features

    * Re-enabled the Alembic caches support for Ogawa archives. HDF5 is not supported anymore inside the FBX SDK.
      Note that all the WindowStore and IOs architectures do not provide Alembic support.

1.2 Changes and Deprecated Features

    * Consider animation corrupted if keys times are not ordered by increase value
      (i.e: T(i) must be less or equal to T(i+1))


2. FIXED AND KNOWN ISSUES
-------------------------

2.1 Fixed Issues

    [FBXX-1499] Fix potential crash when writing Acclaim file.
    [FBXX-1483] Security Improvements.
    [FBXX-1481] Fix potential crash in FbxGeometryConverter::ComputeEdgeSmoothingFromNormals
    [FBXX-1480] 3DS Reader returns a false positive error.
    [FBXX-1479] FBXCollada improve write time of mesh data.
    [FBXX-1476] FbxIO::GetTmpProjectName and FbxIO::SwapFromTmpProject bad execution.
    [FBXX-1474] Password protected files status is overwritten.
    [FBXX-1433] Update Alembic to version 1.7.5.
    
2.2 Known Issues


3. RELEASE NOTES FROM PREVIOUS RELEASES
---------------------------------------
2019.1
    * Added the FBX standardized renaming strategy to the Collada file read/write. This renaming
      strategy will replace every character in the object names/ids, which does not comply with
      the XML NCName scheme, with a string in the form "FBXASC###" were ### is the decimal value
      of the ASCII character.
    
    * Added extra parameter to GenerateTangentsData() family functions to skip testing for the tangent flip

    * moved the static functions FbxRenamingStrategy::NoPrefixName to FbxRenamingStrategyUtils::NoPrefixName
      and it now returns an FbxString instead of a char*
    
    * Starting with this version of the FBX SDK your programs must explicitly link with the zlib and 
      libxml2 libraries. On Linux and MacOS, these libraries should already be part of the system. If 
      not, you must install them.
    
    * disabled Alembic support
    
    [FBXX-1438] Fixed EvaluateGlobalBoundingBoxMinMaxCenter so it correctly returns the 3D volume bounding box.
    [FBXX-1417] Fixed Wgettimeofday and Wgetlogin already define linker error.
    [FBXX-1383] Fixed stack overflow when importing Collada files defining external references.
    
2018.1.1
    * [FBXX-1384] Regression issue introduced by the Audio transport feature.
    * [FBXX-1386] Copying FbxSkin object into another does not copy all data.
    
2018.1
    * [FBXX-1258] Add Audio transport support. 
    * [FBXX-1279] Added more options to the FbxSceneCheck utility class.

2018.0

    * [FBXX-1303] Crash when deleting NURBS created with FbxGeometryConverter::ConvertPatchToNurbsSurface()
    * [FBXX-1280] Invalid pointer access
    * [FBXX-1257] Invalid value for a malloc
    * [FBXX-1143] Collada reader
    * [FBXX-1140] Crash in FbxGeometryConverter::Triangulate with huge poly count (60 millions +)
    
2017.1.1

    Fixed some global HDF5 symbols that were missed during the previous namespacing. These
    symbols only caused an issue on Linux builds when an application links with both the
    FBX SDK and its own version of Alembic/HDF5 libraries static libraries.

2017.1

    * [FBXX-1225] Security Improvements.
    * Alembic code has been fully namespaced to avoid duplicated function conflicts when applications
      link the FBX SDK and other Alembic libraries.

2017.0.2

    * [FBXX-1207] FBX SDK library for VS2015 builds with the Target Platform Version 10.0.10586.0

2017.0.1

    * [FBXX-999] Added the new eRelativeRepetition flag to the animation curves extrapolation flags.    
    * [FBXX-1023] Refactored FbxUtils to minimize possible buffer under/overflows.
    * [FBXX-1031] Added new roll bones to the FbxCharacter definition.
    * [FBXX-1066] The SDK version is still reported as an "Alpha" release
    * [FBXX-1019] Loading a .3ds file with a texture is very slow when using Python bindings
    * [FBXX-1026] Crash when triangulating NURBS surfaces
    * [FBXX-1051] In some cases shape normals are written beyond array boundaries
    Removed deprecated functions
    
        FbxObject::Is(const FbxClassId& pClassId)
        FbxObject::DisconnectAllSrcObject(FbxClassId pClassId)
        FbxObject::GetSrcObjectCount(FbxClassId pClassId)
        FbxObject::GetSrcObject(FbxClassId pClassId, int pIndex=0)
        FbxObject::FindSrcObject(FbxClassId pClassId, const char* pName, int pStartIndex=0)
        FbxObject::DisconnectAllDstObject(FbxClassId pClassId)
        FbxObject::GetDstObjectCount(FbxClassId pClassId)
        FbxObject::GetDstObject(FbxClassId pClassId, int pIndex=0)
        FbxObject::FindDstObject(FbxClassId pClassId, const char* pName,int pStartIndex=0)
    
        FbxProperty::DisconnectAllSrcObject(const FbxClassId& pClassId)
        FbxProperty::GetSrcObjectCount(const FbxClassId& pClassId)
        FbxProperty::GetSrcObject(const FbxClassId& pClassId, const int pIndex=0)
        FbxProperty::FindSrcObject(const FbxClassId& pClassId, const char* pName, const int pStartIndex=0)
        FbxProperty::DisconnectAllDstObject(const FbxClassId& pClassId)
        FbxProperty::GetDstObjectCount(const FbxClassId& pClassId)
        FbxProperty::GetDstObject(const FbxClassId& pClassId, const int pIndex=0)
        FbxProperty::FindDstObject(const FbxClassId& pClassId, const char* pName, const int pStartIndex=0)

2016.1.2

    * [FBXX-1009] StringList default values not correctly initialized (regression in 2016.1).
      Properties created with the FbxStringListDT data type were failing to initialize their default
      value since the implementation of [FBXX-946]

2016.1.1

    * Added the GetMatrix(FbxAMAtrix& pMatrix) function to the FbxAxisSystem class that retrieves
      the axis definition in the 4x4 matrix form.

    * [FBXX-997] FBX SDK Sample ExportScene05 uses a "reserved" (ABC) file extension.
      the ABC extension being also used by the Alembic reader, the sample was selecting
      the wrong reader when trying to load the custom file. Changed the extension of the
      sample so no more conflict can occur. Note that the ABC extension can still be used
      but a manual detection of the readerID, by calling FindReaderIDByDescription(),
      is mandatory.

      
    * [FBXX-988] Resolved use of arbitrary up vector on cameras driven by a target. 
      The FbxAnimEvalClassic::ComputeGlobalTransform() was implicitly assuming in its calculations,
      when the pApplyTarget is true, that the global up vector of cameras constrained by an aim 
      target is always (0,1,0). This was a false assumption if a different FbxSystemAxis had been 
      applied to the scene and, at best, produced an arbitrary rotation of these cameras when the 
      aim vector was parallel to the (0,1,0) vector.
      
    * [FBXX-986] Fixed potential security issue when reading certain legacy FBX files


2016.1

    * [FBXX-967] Improved the LodGroup object to allow it to store thresholds
      as percentage values or distance from camera.
               
    * [FBXX-946] Properties with the FbxEnumDT data type now perform a strict
      validation to forbid duplicates when items are added, inserted or replaced
      in the enumeration list. The new eFbxEnumM type identifier is available for
      users who want to create their own runtime enumeration data type that can 
      accept duplicated entries.
    
    * [FBXX-963] In the Collada reader, replaced xmlParseFile() function call with 
      xmlReadFile() so we can set XML_PARSE_HUGE flag, which relaxes any hard-coded
      limit from the parser. This affects limits like maximum depth of a document or
      the entity recursion, as well as limits of the size of text nodes.

    * [FBXX-952] Fixed an issue with the Animation Curve Pre/Post Extrapolation
      data. In the very particular case of a program creating an animation curve
      directly in memory that set the extrapolation flags and then tries to evaluate
      the curve outside the animation range, the extrapolation flags were not 
      considered until the curve was saved to disk.
     
    * [MAYA-54538] An overlooked condition from the fix above introduced a regression
      in the FBX SDK 2016.1 alpha.

    * [MAXX-23051] An update to the FbxAMatrix::GetROnly() caused a regression when 
      detecting the Euler gimbal locks while extracting the rotation vector value. 
      The default threshold value used for this detection can be to small for a 
      correct detection if the matrix is constructed with very low precision single
      floating point values. 
      The FBX SDK now provides a method (FbxEuler::SetDegenerateThreshold()) to allow
      the client application to change the threshold value at runtime.

2016.0

    * Visual Studio 2008 builds are no longer supported. Visual Studio 2010,
      2012 and 2013 are supported.
    
    * Updated file format to 7.5 (compatible with 2016+ products) which now
      uses 64bit integers in the binary block headers to allow for large file
      support (>2GB). These new FBX files will NOT import in older products
      (2015 or less), so make sure you set the right export format if you need
      to target these older products.
    
    * iPhone Simulator build is now a universal binary that contain both 32 and
      64bit architectures.
    
    * Starting in this release, the FBX SDK name space will no longer contain
      version indications.
    
    * Improved the camera manipulator speed when very close to camera interest
      position.
    
    * Added RemoveBadPolygonsFromMeshes to FbxGeometryConverter class to allow
      clean-up whole scene (typically, this function must be called before
      Triangulate or any other utility functions that are manipulating meshes).

    * Default export file format is now 7.5 (compatible with 2016+ products).
    
    * Removed deprecated functions
        - FbxProperty::SetParent
        - FbxProperty::SetArraySize
        - FbxProperty::GetArraySize
        - FbxProperty::GetArrayItem
        - FbxProperty::operator[]

    * If user code retrieves the transform values directly from the LclTranslation,
      LclRotation and LclScaling properties, the fixes made to the ConvertPivotAnimation 
      may require to change how these property values are retrieved.
      
      Now, when ConvertPivotAnimation is called, the computed value may be stored on 
      the CurveNode channel instead of overwriting the property value. To correctly retrieve
      the computed values, we encourage using the function FbxProperty::EvaluateValue() with the
      time argument value different than FBXSDK_TIME_INFINITE.
      
      Alternatively, users can test for the presence of the CurveNode and retrieve the value
      directly from this object:
      
      ex: if (LclTranslation.GetCurveNode())
            x = LclTranslation.GetCurveNode()->GetChannelValue(0L);

    * Fixed a compilation error when attempting to use FbxAutoDeletePtr on Unix
      based systems.
    
    * Fixed an issue with bounding box calculations.
    
    * Fixed FbxPathUtils::Exist that was failing on MacOS for Sandboxed
      applications.
    
    * Fixed an issue with ConvertPivotAnimation that wasn't properly
      de-activating Rotation pivots/limits after conversion.
    
    * Fixed FbxMesh::RemoveBadPolygons() so that polygons with less than three
      vertices are properly considered invalid.
    
    * Improved CloneManager to avoid duplicating already cloned objects.
    
    * Fix rare crash in FbxScene::ForceKill caused by FbxCharacterPose.
    
    * Fixed an issue with GenerateTangentsDataForAllUVSets that was processing
      tangents for each polygon instead of each mesh when calling
      FbxGeometryConverter::SplitMeshPerMaterial.
    
    * Fixed an issue with ConvertPivotAnimation when processing multiple takes.
    
    * Fixed a crash with ReadUserElementData that was referencing invalid
      pointers.
    
    * Fixed a BVH import rotation issue on joints related to constant key
      reducer.
    
    * Fixed circular dependency in fbxsdk_nsbegin.h.
    
    * Fixed missing template instantiation for FbxSimpleMap and FbxObjectMap.

    * FbxAMatrix only works with XYZ rotation order.

    * Skew and Stretch matrices are not supported.

    * The FBX SDK is *NOT* thread safe.

    * Deformers do not support instancing. This causes point cache deforming to
      not be re-usable.

    * While assigned material can change on the node, material layers on the
      mesh cannot be instanced.

    * When exporting various file format, sometimes the scene gets modified
      permanently, resulting in data loss.

2015.1

    * Added support for Visual Studio 2013 compiler.
    
    * iOS build changes: minimum iOS version is now 7.0 and now supports 3 main
      architectures (armv7, armv7s and arm64).
    
    * MacOS build changes: GCC build support dropped. MacOS minimum version is
      now 10.8, and is built with XCode 5.0 default Clang compiler.
    
    * Native Alembic support added: now .abc files can be directly opened,
      which will create the appropriate nodes and mesh in the FBX scene.
      Afterwards, users can access FbxCache data in order to read cache data.
      NOTE: Alembic support is currently only enabled for desktop operating
      systems (i.e. disabled on iOS/WinStore).
    
    * FbxCache now has a newer function, Read, which works regardless of cache
      type.
    
    * Add .zip read support: files and their dependencies, such as textures,
      can now be packaged into a .zip file, which can be directly read by the
      FBX SDK without unpacking. This is useful for file formats that don't
      support embedding textures. Only one scene per .zip file is supported.
    
    * FbxObject now has a virtual function Compact that can be overloaded, to
      implement object compacting. Calling Compact on FbxScene or FbxDocument
      will compact every objects connected as source.
      
    * FbxArray now provides a AddCompact and Compact function to reduce memory
      used by arrays.
    
    * Added operator equality and inequality to FbxPair.
    
    * Added new class FbxSharedPtr.
    
    * Improved the FbxCameraManipulator behavior:
      - Upon Frame-All, camera rotation is preserved
      - Upon Frame-All, take into account deformations on meshes
      - Make minimum distance to LookAt more robust
      - Improved minimum speed when camera is at minimum distance to LookAt
      
    * Added a new FBX_TRACE macro for debugging.
    
    * Added a new parameter to FbxCamera::ComputeProjectionMatrix to specify if
      the FOV should be preserved even if the viewport height is larger than
      the viewport width.

    * FbxGeometryConverter function ResetMeshesCenterToWorld renamed to
      RecenterSceneToWorldCenter because it doesn't change the vertices data.
    
    * Recent endianness changes were reverted because they are not necessary.

    * Fixed an issue with the matrix converter filter that wasn't resetting all
      members properly.
    
    * Made path utility functions more robust: files are no longer mistaken for
      folders, and trying to write in read-only folders should not crash any
      more.
    
    * The geometry converter Triangulate and SplitMeshesPerMaterials now
      properly handles blend shapes.
    
    * Fixed an issue with left over shape data upon reading FBX files.
    
    * Fixed a major corruption problem when trying to save files in iOS builds.
    
    * FBXSDK_ENV_IOS should no longer be defined on MacOS builds.
    
    * FbxThumbnails are now properly shared between document and importer's
      scene info.
    
    * Converting a scene's axis system with the FbxAxisSystem will now ignore
      pre/post rotation and rotation order values on node if RotationActive
      property is set to false.
    
    * Cleaned-up many warnings when compiling using XCode 5.0.
    
    * Fix a crash with layer element material cloning that was still pointing
      to an not-cloned FbxSurfaceMaterial object.
    
    * Fixed an issue with local time span not always written to FBX file.
    
    * Now when using FbxGeometryConverter::SplitMeshesPerMaterial, all layer
      elements will be copied to the new meshes instead of only the first one.
      
    * Fixed a rare crash upon loading corrupted FBX files.
    
    * Fixed multiple crashes upon loading Collada and OBJ files.

2015.0

    * Added 4 new functions in FbxMatrix to generate LookTo and LookAt
      matrices.

    * Class FbxVertexCacheDeformer now has a type property to define the type
      of data it uses in the cache file channel.
    
    * Added a new generic read function to FbxCache, which returns an array of
      float managed internally. This function now works regardless of the cache
      file type.
    
    * FbxArray now use reference to elements rather than copies for greater
      memory performance.
    
    * Added a new function overload FbxMatrix::GetElements which now can return
      a rotation vector instead of a quaternion.
    
    * Matrix converter curve filter now supports TRS limits.
    
    * Cleaned and standardized comparison functors across the SDK.
    
    * Added operator += and -= to FbxRedBlackTree. FbxMap use this model.
    
    * Added new function FbxArray::RemoveRange that allows removing multiple
      items.

    * Renamed FbxSet to FbxMultiMap to clearly describe what it does.
    
    * Renamed FbxSet2 to FbxSet, since this is the actual set class in FBX SDK.

    * Polished the interface of FbxVertexCacheDeformer class to use the FBX SDK
      standard property model.
    
    * FbxArray will now use reference values for most of its functions.
    
    * FbxPropertyAttr members have been merged into FbxPropertyFlags for
      consistency.

    * Fix a regression with edge smoothing.

    * Fixed multiple memory alignment issues for iOS builds.
    
    * FbxProperty map in evaluate state now properly use an FbxProperty
      reference instead of a pointer.
      
    * Fixed issue with the evaluation state not properly clearing itself when
      the scene changes. This was causing memory corruption.

    * Fixed a crash when freeing the FbxManager in some circumstances.

    * Fixed a crash when importing some Collada files.

    * Fixed a crash when trying to create a folder in read-only locations on
      Linux/Mac.

    * Fixed an issue with tangents smoothing not considering flip.

2014.2.1

    * The change to have the animation evaluator per manager instead of per
      scene has been reverted because it was causing a loss of functionality
      for some applications.
      
    * Now to retrieve the current animation stack, please use
      FbxScene::GetCurrentAnimationStack instead of
      FbxAnimEvaluator::GetContext.

    * FbxProperty::GetCurveNode should once again return the right curve node.
      This caused animation to be completely wrong in some cases.
      
    * Fixed an issue with camera node's LclTranslation property being
      incorrectly overwritten by the camera Position property.

2014.2

    * Added new FBX SDK library targets: Windows Store, Mac Clang and iOS.
    
    * Major improvement to animation evaluation functions:
      - Now, the FbxAnimEvaluator instance is stored in the manager rather than
        the scene.
      - Add evaluation state to animation stacks to allow simultaneous
        evaluation of takes.
      - One evaluation state remains in the evaluator to be able to evaluate
        scene-less objects.
      - Remove the necessity to provide an index to evaluation functions of the
        evaluator. All 'fast' versions of these functions are no longer
        necessary so they have been removed.
      - Added a new function in FbxProperty to evaluate their value.
      - Greatly improve performance of FbxProperty::GetCurveNode by caching
        results in a map inside evaluation state.
      - Greatly reduce the size of a node and property evaluation states.
      
    * Greatly reduced the amount of memory allocations/free that were caused by
      FbxCriteria and FbxQuery. Should improve performance in most cases.
      
    * Added new global function FbxTypeComponentCount to retrieve a type's
      component count.
    
    * Added new functions FbxMapDelete, FbxMapFree and FbxMapDestroy to ease
      the process of deleting map content.

    * New function in FbxTime to retrieve the time string, which allow to
      specify which 'start' and 'end' element it can returns.
      
    * Added a new property to the camera manipulator to allow inverting
      controls.

    * FbxGeometry::ComputeBBox will no longer compute or change the bounding
      box value if the content is unloaded.
      
    * FbxString is now using a char pointer allocation again rather than an
      opaque std::string implementation, for easier debugging and better
      performance/memory footprint. It is no longer necessary to modify the
      autoexp.dat file of Visual Studio to see string content with debuggers.
      
    * More work has been put in to make it possible to clone a character and
      its dependencies. Please see fbxclonemanager.h comments for more info.
      
    * FbxGeometryConverter::SplitMeshesPerMaterial now correctly handles
      skinning on splitted meshes.
      
    * Exposed bounding box calculation function scene and nodes. Please check
      out FbxScene::ComputeBoundingBoxMinMaxCenter and
      FbxNode::EvaluateGlobalBoundingBoxMinMaxCenter.
      
    * Added new function in FbxGeometryConverter to re-center the scene meshes,
      please check out FbxGeometryConverter::ResetMeshesCenterToWorld.
      
    * Added new function in mesh to generate normals, working the same way
      generating tangents, which replace the old function
      FbxMesh::ComputeVertexNormals which only supported smooth normals. Please
      see FbxMesh::GenerateNormals.
      
    * Implemented FbxScene::Clone to properly duplicate scenes.
    
    * Added two new functions to FbxTime: SetFramePrecise and
      GetFrameCountPrecise, which use double values instead of longlong.
      
    * Added FbxMod to fbxmath.h.
    
    * Added a sample code to demonstrate FbxStream class usage.

    * FbxString no longer use std::string implementation.
    
    * FBXSDK_sprintf, FBXSDK_snprintf, FBXSDK_vsprintf and FBXSDK_vsnprintf now
      always take a single size parameter instead of being variant on different
      platforms.
      
    * Removed thumbnails in animation stack. Now only one thumbnail can be
      saved per FBX file, found in the FbxSceneInfo class.
      
    * Removed GetCurveNode in FbxProperty that used an animation stack name for
      performance reasons.
      
    * Changed ConvertPivot functions in FbxNode to use an animation stack
      pointer instead of animation stack for performance reasons.
      
    * Renamed class FbxPropertyFlagsExt to FbxPropertyFlags.
    
    * Moved class FbxPropertyValue to a more global locations because it is now
      also used by evaluation state.
    
    * When using the camera manipulator, and the camera got flipped vertically,
      the horizontal orbit was inversed. This has been fixed.
      
    * Fixed an issue with matrix conversion to euler angles; degenerate case
      (gimbal lock) detection was too sensitive.
      
    * ContentUnload will now properly unload layers on geometry.
    
    * When performing a 'frame-all' using the camera manipulator, if the mesh
      content is unloaded, it will use the pre-computed bounding box, if it
      exist, otherwise it will skip it.
      
    * When performing a 'frame-all' using the camera manipulator, the camera
      will now correctly center itself on the global bounding box center rather
      than its centroid.
      
    * Fixed many compilation warnings with clang compiler on Mac.
    
    * Fixed camera orthogonal projection matrix generation issue.
    
    * When manipulating camera, it shall no longer go through the interest
      position.
      
    * Fixed crash when writing encrypted FBX file.
    
    * Fixed crash when opening OBJ files.
    
    * Fixed typo with return value of FbxExporter::GetCurrentWritableVersions.
    
    * FbxCameraManipulator Frame-All now works on orthographic cameras.
    
    * Fixed edge detection to be more accurate. Previous code only stored one
      edge from a single point. Now save all edges from one point and detect
      duplicated edges in all situation.
      
    * Fixed various warnings in macros generated when compiling with C++11.
    
    * Fixed an issue when reading textures for OBJ files.
    
    * Fixed an issue when reading material transparent properties in MTL files.
    
    * Fix rare crash when reading 3DS files.
    
    * Fixed rare memory leaks in specific situations.
    
    * Fixed reversed normals when reading DXF files.
    
    * Fixed a crash when writing collada files containing shapes.
    
    * Fix function to generate tangents/binormals incorrectly sets W component
      to greater values than -1 or 1.
      
    * Cameras with a target up node or a look at node will now be able to be
      moved around properly using the camera manipulator.
    
2014.1

    * Introduced FBXSDK_ENV_WINRT preprocessor define (previously known as
      FBXSDK_ENV_WINMETRO) which disable unsupported functions when targeting
      Microsoft Windows Store applications. This define will automatically be
      defined if you set Windows Store App Support to Yes in your Visual Studio
      project settings.

    * A number of changes to the architecture defines in fbxarch.h were changed
      to add support for ARM processor:
          Added FBXSDK_ARCH_IX86, only defined on Intel x86 targets.
          Added FBXSDK_ARCH_AMD64, only defined on AMD64 targets.
          Added FBXSDK_ARCH_ARM, only defined on ARM targets.
          Renamed FBXSDK_ARCH_32 to FBXSDK_CPU_32, only defined for 32bit targets.
          Renamed FBXSDK_ARCH_64 to FBXSDK_CPU_64, only defined for 64bit targets.
      Also, please see deprecated details below.
      
    * Added version information to the FBX SDK dll on Windows platform.
    
    * Updated ViewScene sample to show how to support in-between blendshape
      (progressive morph).
      
    * Added support for the new Maya 2014 Cache files (*.mcx).
    
    * Improved the tangents/binormals calculation functions to properly handle
      mirrored faces by flipping the binormal when the face is inverted. It
      also sets the fourth component of the tangent to -1 if it is flipped, 1
      otherwise.
      
    * FbxReference is now defined as FbxObject* rather than void*. This will
      allow properties of FbxReference type to correctly respond to FbxCast and
      other FBX SDK API, as well as restraining the assignment to FbxObject*
      types.
      
    * Improved the FbxCameraManipulator by adding a new action type: eFreePan.
      It combines dollying and panning into a single action. For example, this
      is very useful for manipulating the camera using a touch screen.
      
    * Added a new object flag: FbxObject::eInitialized. This flag is
      automatically set on all object after they are done with the Construct
      stage. This is also used to prevent PropertyNotify to be called during
      ConstructProperties stage.
           
    * Now the class FbxCameraManipulator does not use an hidden class
      internally anymore, and use the PropertyNotify event to re-initialize
      when changing its affected camera.
      
    * Added a new IOSetting flag (IMP_FBX_EXTRACT_EMBEDDED_DATA) to completely
      disable embedded files extraction, which is set to true by default.
      
    * Added a new function (FbxObject::ResetProperties) to reset all properties
      to their default value.
      
    * The parameter pForceSet of function StaticInit on FbxProperty is not
      optional anymore. When calling it in ConstructProperties, the value of
      pForceSet must be correctly passed to StaticInit for all properties to
      ensure properties are not initialized more than once.
      
    * Added new function FbxGeometryConverter::Triangulate which now replace
      the old TriangulateMesh, TriangulateMeshAdvanced and TriangulateInPlace.
      The new function can either take a node attribute, or a scene in
      parameter. It can also replace original nodes with new ones and make sure
      the scene stays clean.
      
    * Added new function FbxGeometryConverter::SplitMeshesPerMaterial which can
      take a scene in parameter, and can replace original meshes and make sure
      the scene stays clean.
      
    * Added a new constructor to FbxArray class, which takes an integer in
      parameter to reserve the memory for that specific size.
      
    * Added a new parameter to FbxMesh::GetPolygonVertexUV to allow it to return
      an array of unmapped polygons. This is extremely useful to determine which
      vertex has no UV associated.
      
    * Removed the version number in library names on disk for all platforms.
      Also, debug and release versions are now named the same, but in different
      subfolders.

    * FBXSDK_ARCH_32 and FBXSDK_ARCH_64 got renamed to FBXSDK_CPU_32 and
      FBXSDK_CPU_64 respectively to dissociate them from the architecture.
      
    * Cannot set void* types to FbxReference properties anymore to improve
      robustness.
      
    * Changed the signature of FbxObject::PropertyNotify to pass a reference to
      the property instead of a pointer.
      
    * Old triangulation functions are now deprecated. Please use
      FbxGeometryConverter::Triangulate instead.

    * Improve manipulation of free cameras by preventing nasty flips when view
      axis reaches vertical.
      
    * A regression with ConvertPivotAnimationRecursive that was introduced in
      FBX 2013.3 has been fixed. It was causing some scenes to become resampled
      and key-reduced wrongly.
      
    * When including fbxsdk.h in your project, the define WIN32_LEAN_AND_MEAN
      will not be automatically defined anymore.
      
    * Fixed an issue with FbxPathUtils::GetRelativePath that wasn't working
      properly with localized paths.
      
    * Fixed a crash when exporting to 2010/2011 compatible files (FBX 6.1/7.1).
    
    * Fixed a performance issue with Quaternion FCurve evaluation.
    
    * All properties of FbxReference type now use the Source connection. Only a
      handful of them weren't complying, in various FbxConstraint classes. In
      FBX 2014 (7.4) or lower files, this will not change anything, but in
      memory these objects are now always connected to the source of the
      property.

    * Blend Shape animation was missing when a take other than first is
      imported, this has been corrected.
      
    * FbxObject::PropertyNotify callback will not be called anymore during
      ConstructProperties object creation stage.

    * FbxPropertyT<FbxReference> now correctly receive the PropertyNotify event.
    
    * Fixed an issue with FbxTime::SetTimeString when passing spaces between
      elements.
      
    * Fixed an issue with the FbxCamera Position property synchronization with
      its node's local translation.
      
    * Fixed FbxMarker class to not return property pointers.
    
    * For all FbxMesh::GenerateTangentsData function: When the tangents data
      already exist on the mesh, and that pOverwrite is set to false, the
      function will now return true so that it is not considered as an error.
      
    * Added a notification in the doxygen documentation of FbxStream that it
      does not support other formats than FBX.
      
    * Fixed a crash in FbxCache evaluation under Linux/Mac platforms.
    
    * Fixed a rotation order issue with ASF motion files.
    
    * Fixed a candidate value initialization with ASF motion files.
    
    * Fixed a crash on Mac/Linux version in FbxPathUtils::Create.
    
    * Made FbxGeoemetryConverter::SplitMeshPerMaterial much more robust; now
      meshes that have material indexes that points to invalid materials will
      not be assigned any materials, and they will be regrouped together.
      
    * Fixed a crash when exporting blend shapes to an old FBX 6.x file format.
    
    * Fixed an issue with tangents/binormals generation when some vertices
      didn't have an UV mapped.
      
2014.0

    * Visual Studio 2012 is now officially supported.

    * FbxCamera::EvaluateUpDirection now support the Roll property.

    * Added FBX 2014 file version, and his now the default target.

    * The 4th vector element of normals, binormals and tangents are now
      properly exported to FBX files in FBX 2014 file format.
      
    * A new status reporting class has been introduced to replace the old
      FbxError class. Please refer to the FbxStatus class. More usage of this
      new class will be added over time.
      
    * Added new functions for the default IK Pull value on markers.
    
    * A default IK Pull value is now stored on markers.
    
    * TimeMarkers functionality has been re-introduced in FBX 2014 file format.
      The API to access them has been placed in FbxGlobalSettings class.
      
    * Updated Python 3.1 support to Python 3.2 in the FBX Python SDK.
    
    * MotionFiles readers/writers are now registered in the public FBX SDK.
      This means the FBX SDK can now import and export the following motion
      file formats: Biovision (BVH), MotionAnalysis (HTR/TRC), Acclaim (ASF/AMC),
      Vicon (C3D), Adaptive Optics (AOA) and Superfluo (MCD).
      
    * The FBX SDK libraries on Mac and Linux are now compiled with default
      visibility set to hidden. Only symbols marked with FBXSDK_DLL are exported,
      which makes it equal to the Windows version.
      
    * FbxProperty flags have been clean-up. The flag ePublished as been renamed
      to eAnimated, and eUser to eUserDefined, for clarity. The flag eStatic
      now works, and will be turned on for all static properties by default.
      Introduced the flag eImported, for properties that are created upon
      import.
      
    * New environment define FBXSDK_ENV_WINRT for Windows Metro support. Also
      added FBXSDK_ARCH_ARM for ARM processor support, under work.
      
    * FbxFolder has been re-worked. FbxFolder::Create, FbxFolder::Delete and
      FbxFolder::Exist has been moved in FbxPathUtils, and now work pass the
      Microsoft Certification Validation Kit for Windows Store applications.
      Other functions of FbxFolder have been renamed for consistency. Please
      see fbxfolder.h.
      
    * New function FbxPathUtils::Exist now works on relative paths.
    
    * New function FbxPathUtils::Create can take multiple paths as argument,
      and they will all be created (if possible), just like mkdir on Unix.
      
    * Improved FbxMesh::GetPolygonVertexCount to simply return the array size.
    
    * Updated FbxProgress class to be simpler, and allow an additional argument
      to be passed to the single-thread callback.

    * The file fbxsdk_compatibility.h has been removed from the FBX SDK
      distribution, so now it is required to use the latest API. Defining
      FBXSDK_NEW_API is not necessary anymore and should be removed from your
      projects.

    * The support for Visual Studio 2005 has been discontinued.

    * The support for MacOS/X SDK 10.5 has been discontinued. The minimum
      requirement is now MacOS/X SDK 10.6.

    * FbxString::Empty was renamed for FbxString::Clear to avoid confusion with
      FbxString::IsEmpty.
      
    * FbxError has been removed from all classes. Instead, functions that needs
      to return an error string now use an optional parameter of type FbxStatus
      similar to the MStatus in Maya API.
      
    * FbxFolder::Create has been removed, please use FbxPathUtils::Create
      instead.
      
    * FbxFolder::Delete has been removed, please use FbxPathUtils::Delete
      instead.
      
    * FbxFolder::Exist has been removed, please use FbxPathUtils::Exist instead.
      
    * FbxFolder::Remove has been removed from the FBX SDK API. Please now use
      FbxPathUtils::Delete, which is now always recursive.
      
    * Fixed a small issue with FbxCamera::EvaluateUpDirection that was
      incorrectly using the TargetUp object.
    
    * Updated various doxygen comments for clarity.

    * Fixed a buffer overrun crash issue when importing specific 3DS files.

    * Fixed a crash issue when trying to destroy the FbxManager at application
      exit.

    * Some samples were erroneously calling FbxString::Empty instead of
      FbxString::IsEmpty.

    * Fixed a buffer overflow in FbxStream::ReadString.
    
    * Importing an OBJ file will not un-weld UV anymore.
    
    * Fixed curve key comment in ASCII version; the code was not distinguishing
      between the GenericTimeIndependent only and GenericClampProgressive.
      
    * Improved the TranslationActive, RotationActive and ScalingActive
      properties doxygen documentation.
      
    * Fixed a crash when exporting to Collada when the scene was containing
      other kinds of deformers than skins.
      
    * The default value of FbxMarker::DrawLink is now set to true.
    
    * Fixed a small issue with FbxCamera::EvaluateLookAtPosition that wasn't
      returning the expected value.
      
    * Fixed an issue when writing Collada files that was resulting in the
      Effect field being written for each polygon, wrongly increasing the file
      size.
      
    * Various functions, such as FbxMin, that were returning a const reference
      type value were fixed to return a copy of the value instead.
      
    * Various function parameters that were asking for const reference intrinsic
      type were changed to ask a copy of the intrinsic type (which is faster
      than a const reference in this case).
      
    * Fixed a small typo error in the Transform sample code.
    
    * Small optimization on connection management for large scenes.
    
    * Fixed a crash when calling FbxScene::ForceKill on a scene imported which
      contained thumbnails.
      
    * Made fbxiosettingspath.h header file compatible with C++x11 standard.

2013.3

    * The class FbxCameraManipulator has been improved so that the FrameAll and
      FrameSelected functions now take deformations into account, and also
      properly adjust the camera distance for a better framing.
      
    * Removed the FbxNode_internal class. Now all attributes of FbxNode are
      visible within the FbxNode class declaration itself. This caused some
      changes to the Pivot informations.
      
    * Now the FbxCache using IFF component can scale to 64bit, allowing for
      much larger cache files to be used.

    * The dynamic library version of the FBX SDK library on the Windows
      platform now use the static MSVCRT (/mt) rather than the dynamic MSVCRT
      (/MD). This allows to distribute applications using the FBX SDK dll
      without having to install the Microsoft Visual Studio Redist.
      
    * From now on, all classes that previously overloaded Clone will now only
      clone itself, and not its sources. For instance, cloning an FbxNode will
      NOT clone its FbxNodeAttribute anymore. Instead, to reproduce this
      behavior, one must use the FbxCloneManager which handles cloning object
      networks. This now holds true for most objects of the FBX SDK. However,
      objects such as meshes with their skins and clusters remain considered as
      one entity.
      
    * The class FbxScene now overload the Clone function, and uses the
      FbxCloneManager to successfully clone the entire scene network.
      
    * The FBX SDK now use default memory allocation functions when they are
      called without being initialized. This only happens when static variables
      are initialized before main. If you need to change the FBX SDK memory
      allocation functions, make sure you are not using static FBX SDK objects
      that could potentially be initialized before your main call.
      
    * Added new functions to retrieve the memory allocation function currently
      set, as well as the default ones. Please see fbxalloc.h file.
      
    * Added support for 'locked' flags on properties. This won't prevent FBX
      SDK users from changing property values, but it will allow applications
      to store/retrieve the flags across. Since properties can have up to 4
      members, we're using 4 bits to differenciate them.

    * Fixed an issue when importing files with bump textures using foreign
      languages.
      
    * Added a bit more details to the installers detail pane on Windows.
    
    * Fixed a crash with the Geometry Converter; duplicated edges are now
      correctly detected.
      
    * Fixed a minor issue in the ViewScene sample, concerning holes in the
      mSubMeshes table.
      
    * A lot of classes using direct pointers to objects have been corrected to
      use reference properties, which makes the SDK more robust.
      
    * Properties that have the NotSavable flag now do not write their
      connections, thus making FBX files smaller.
      
    * The ShadingModel property on FbxSurfaceMaterial inherited classes will
      only be overwritten for unknown types.
      
    * Clean-up remaining cases where the specified memory allocator wasn't
      correctly used internally in the FBX SDK.
      
    * Several classes needed an update to their ::Copy method so that all
      elements are correctly copied.
      
    * Fixed the MatrixConverter filter so that it doesn't get fooled by the
      LeftHand system if the input animation has a negative scaling vector.
      
    * Fix several issues with CharacterPose reading/writing mechanics.
    
    * An invalid FBX reader/writer was chosen when opening older FBX files
      (5.x/6.x) using FbxStream. Now the correct reader/writer are instanced.
      
    * Fixed various warnings with FBX SDK samples on Mac/Linux.
    
    * Fixed a crash when attempting to initialize objects before main, caused
      by uninitialized memory allocation functions.
      
    * Fixed an issue with the tangents/binormals generation in the smoothing
      phase not taking into account UV seams. Now the smoothing should not
      cross UV seams boundaries.

2013.2

    * Basic Lambert and Phong material properties are now animatable.
    
    * Installation paths for all FBX products is now consistent. For instance,
      the FBX SDK will now install in ".../FBX/FBX SDK/20xx.x".
      
    * Added support for FBX Python SDK on Linux 32bit.
    
    * Added new function to evaluate camera target up, that will take into
      account the Target-Up object position specified on the Node.
      
    * Added new parameter to Readers and Writers registration process so that
      now it is possible to override one, rather than just add it at the end of
      the list.
      
    * Removed dependency to WININET library on the Windows platform. Now, when
      linking with the FBX SDK, no other dependency are generated. All samples
      have been updated to reflect this.
      
    * The FBX SDK library is now roughly ~10% smaller, since a lot of obsolete
      code has been removed.
      
    * The constructor and destructor mechanics have been standardized across
      the whole SDK. Now, the required constructor when declaring a class that
      inherit from FbxObject is defined by default in the FBXSDK_OBJECT_DECLARE
      macro. If you need to execute code in the constructor or destructor, you
      will need to override Construct and Destruct as detailed in the
      documentation.
      
    * The virtual function ConstructProperties signature now do not have any
      return value. please update your code accordingly. As a reminder, please
      do not forget to call ParentClass::ConstructProperties at the beginning
      if you intend to override it.
      
    * FbxRedBlackTree has been moved into its own file, which makes it easier
      to read fbxmap.h now.
      
    * The class FbxScopedPtr has been renamed FbxAutoPtr and has been moved
      in fbxalloc.h so it is accessible by default. Also, a few basic overloads
      with different deletion policy have been implemented. Those are
      FbxAutoFreePtr, FbxAutoDeletePtr and FbxAutoDestroyPtr.
      
    * All functions of the FBX SDK that used to take a ClassId or a FBX_TYPE
      macro as input for filtering are now deprecated. Now, the templated
      version of the same function name must be used. Since it is templated,
      they now return the correct value type, and it is no longer necessary to
      call FbxCast on the return value.
      
    * Fixed an issue when specifying "ApplyTarget" with the classic evaluator.
      
    * The tangents/binormals generation function in the FBX SDK has been
      updated and now works on polygons with more than three sides.

    * The class FbxExposureControl has been retired.
    
    * The class FbxPlug has been retired.
    
    * The function FbxProperty::GetCurve is not templated anymore.
    
    * The function FbxObject::GetRootProperty is now deprecated. Please use the
      member FbxObject::RootProperty instead.
      
    * The function FbxCamera::GetUpVector is now deprecated. Please use
      FbxCamera::EvaluateUpDirection instead.
      
    * The function FbxCamera::ComputePerspective is now deprecated. Please use
      FbxCamera::ComputeProjectionMatrix instead.
      
    * Deprecated functions in fbxcompatibility.h file have been removed.
    
    * The class FbxPointerGuard has been retired, because FbxAutoPtr and its
      derivates replaces it.
      
    * The function FbxArray::SetCount has been retired. Please use
      FbxArray::Reserve or FbxArray::Resize instead.
      
    * The function FbxArray::Empty is now deprecated. Please use
      FbxArray::Clear instead.
    
    * The function FbxArray::FindAfter is now deprecated. Please use
      FbxArray::Find instead.
      
    * The function FbxArray::FindBefore is now deprecated. Please use
      FbxArray::FindReverse instead.
      
    * The function FbxArray::AddMultiple is now deprecated. Please use
      FbxArray::Grow instead.
      
    * All functions in FbxObject, FbxProperty, FbxCollection and FbxDocument
      that used to take a FbxClassId or an FBX_TYPE() macro, as the filtering
      mechanism, are now deprecated. Please use the templated version of these
      functions, such as: FbxObject::GetSrcObjectCount<Type>() or
      FbxObject::GetSrcObject<Type>() The templated version returns the correct
      type, so it is no longer needed to use FbxCast upon return of these
      functions.
      
    * All functions in FbxObject and FbxProperty that used to take a FbxClassId
      plus an FbxCriteria have been retired instead of being deprecated,
      because they didn't make sense from a logical standpoint, and can now
      safely be replaced with the form Obj->Function<Type>(Criteria).
      
    * The following functions have been made deprecated, and should be replaced
      with their equivalent found in FbxObject:
        FbxConnectSrc(Dst, Src)               -> Dst->ConnectSrcObject(Src)
        FbxConnectDst(Src, Dst)               -> Src->ConnectDstObject(Dst)
        FbxGetSrcCount(Obj)                   -> Obj->GetSrcCount()
        FbxGetSrcCount(Obj, ClassId)          -> Obj->GetSrcCount<Type>()
        FbxGetSrc(Obj, Index)                 -> Obj->GetSrcObject(Index)
        FbxGetSrc(Obj, Index, ClassId)        -> Obj->GetSrcObject<Type>(Index)
        FbxFindSrc(Obj, Name, Index)          -> Obj->FindSrcObject(Name, Index)
        FbxFindSrc(Obj, Name, ClassId, Index) -> Obj->FindSrcObject<Type>(Name, Index)
        FbxDisconnectAllSrc(Obj)              -> Obj->DisconnectAllSrcObject()
        FbxGetDstCount(Obj)                   -> Obj->GetDstCount()
        FbxGetDstCount(Obj, ClassId)          -> Obj->GetDstCount<Type>()
        FbxGetDst(Obj, Index)                 -> Obj->GetDstObject(Index)
        FbxGetDst(Obj, Index, ClassId)        -> Obj->GetDstObject<Type>(Index)
        FbxFindDst(Obj, Name, Index)          -> Obj->FindDstObject(Name, Index)
        FbxFindDst(Obj, Name, ClassId, Index) -> Obj->FindDstObject<Type>(Name, Index)
        FbxDisconnectAllDst(Obj)              -> Obj->DisconnectAllDstObject()

    * Fixed an issue with scaling inheritance not being computed properly by
      the classic evaluator.
      
    * The tangents and binormals generated by the FBX SDK will now be smoothed
      according to the normal smoothing.
      
    * Optimized tangents/binormals generation so that there not more of them
      than there is normals on the mesh.
      
    * The FBX SDK release version can now be safely linked into a debug app.
    
    * Fixed a texture indice issue with old FBX 5.x content.
    
    * Fixed a memory leak with runtime classes.
    
    * The FBX SDK now uses the memory allocator in the FbxString class for all
      allocations done by std::string.
      
    * Changed the behavior of GetAnimationInterval so that it doesn't do a
      union anymore. Now, the timespan parameter is always initialized properly
      inside the function.
      
    * Improved how the FBX SDK handle testing if file exist, resulting in a
      faster file opening for missing assets that are over the network.
      
    * Fixed a file crash when importing an FBX 5.x file without setting an IO
      setting.
      
    * Fix some issues with samples, as reported by users.
    
    * Resolved issues with the memory allocator. For instance, FbxString now
      use the specified memory allocator rather than the default STL allocator.
      
    * Fixed a crash with a specific OBJ file.
    
    * An issue with ResetPivotSetAndConvertAnimation breaking instanced mesh as
      been corrected.
      
    * Fixed multiple FBX Python SDK errors in samples.
    
    * Fixed an issue with Collada export for meshes with multiple materials.

2013.1

    * Renamed all classes with prefix "Fbx". Also renamed all structures with
      the same prefix.
      
    * Global functions and enumerations now start with the "Fbx" prefix.

    * By default, the file fbxsdk_compatibility.h will be included when
      including fbxsdk.h. This file defines a lot of the old class names to
      help with the transition. However, if you want to use the new API, you
      can define FBXSDK_NEW_API in your project and the compatibility file will
      not be included, but will most likely result in a lot of errors to fix
      in your application.
      
      It is highly recommended that you start by fixing compilation errors
      before defining FBXSDK_NEW_API in your project, but it is also highly
      recommended to fully fix your code since the compatibility file might
      be removed in a future release.
      
    * Moved and renamed all enumerations into classes when possible.
    
    * Moved files into appropriate folders, and grouped them.
    
    * Removed KFbxMemoryAllocator class, instead use handlers setters functions
      found in fbxalloc.h, such as FbxSetMallocHandler.
      
    * Removed all KFCurveFilter classes, please use FbxAnimCurveFilter classes
      instead.
      
    * FbxFile is now exposed! See fbxfile.h for the list of available
      functions.
      
    * A new class, FbxFileUtils, now expose all static functions related to
      file handling, such as remove or rename, etc.
      
    * A new class, FbxPathUtils, now expose all static functions related to
      file path handling, such as IsRelative or Clean, etc.
      
    * Completely re-designed how Character Poses are stored in FBX.
    
    * The FbxLight class has been augmented to support area lights and barn
      doors.
      
    * The FbxGeometryBase class has been augmented to support render options
      such as PrimaryVisibility, CastShadow and ReceiveShadow.
      
    * Many FBX SDK functions that were asking for file paths as inputs were
      updated to support UTF-8. Their parameter names were updated to reflect
      this change.
      
    * It is now possible to retrieve the "long" version string of the FBX SDK
      via the function FbxManager::GetVersion(true). This will allow developers
      to get the version string of this library along with the name and the
      revision number.

    * Fixed various issues with sample codes.
    
    * Now sample codes compile with warning level 4.
    
    * Fixed an issue with animation evaluation that wasn't updated correctly
      when told to.
      
    * Fixed an issue with constraints being lost after export.
    
    * Fixed an issue with animation evaluation sometimes returning twice the
      value amount.
      
    * Fixed an issue with RemovePolygon function on mesh class when mapping is
      by polygon.
      
    * Initial property values of LclTranslation, LclRotation and LclScaling
      affect the evaluator's result when there are actually animation curves
      connected to these properties. This has been corrected.
      
    * Fixed an issue with writing cache data on Mac when a space was present
      in the file path.
      
    * Fixed a crash with xstring use in VS2010, caused by an assert.
    
    * Improved precision in time and timecode classes.
    
    * Changed how absolute and relative paths are handled during import. Now
      if one of the two path is opening successfully, the other is changed to
      work.
      
    * Fixed an issue with the unroll animation filter.
    
    * Now, when resampling animation, we move the default at the end rather
      than at the start of the resampling.

2012.2

    * Added Area Light support to KFbxLight class.

    * Added Barn Doors support to KFbxLight class.

    * HotSpot property on KFbxLight has been changed for InnerAngle.
    
    * ConeAngle property on KFbxLight has been changed for OuterAngle.

    * Various file I/O crashes.
    
    * Duplicate material issue with material converter is fixed.
    
    * Some minor fixes in sample codes.
    
    * Fixed path issue in Collada reader.
    
    * After writing FBX file, the scene should come back to its initial state.
    
    * Fixed a crash on import of old scenes when it didn't contain blendshapes
      or morpher data.
      
    * Fixed issue with incorrect normals when converting from OBJ to FBX.
    
    * Sample code "Cube Creator" was modifying the scene when exporting in OBJ,
      This has been fixed.
      
    * Fixed an issue with absolute versus relative file paths; now if one of them
      is valid and the other is not, we fix the broken one.
      
    * Fixed an issue with cache writing on Mac when there was a space in the file
      name.
      
    * Fixed an issue with the FBX exporter that would duplicate materials in very
      specific conditions.
      
    * After writing an FBX 6.x file with shapes, the scene was modified and data
      was potentially lost. This has been corrected.
      
    * Fixed an assert caused by stl string in VS2010.
    
    * We now open FBX files in read-only mode rather than read+write. This allow
      for other process to access the FBX file in parallel.

2012.1

    * Added support for Image Sequences into KFbxVideo.
    
    * Improved the reference documentation quite substantially.

    * Refined how visibility inheritance works: added a new property for each
      node allowing to specify if it should be inherited or not.
      
    * Added a new option in IO Settings to import the time span found in FBX
      files.
      
    * Added a new function to KFbxAxisSystem to retrieve the front vector.
    
    * Added a new function to KFbxAnimEvaluator and KFbxAnimEvalClassic to
      allow users to calculate the local TRS from the global, with an offset.
      See ComputeLocalTRSFromGlobal in kfbxanimevaluator.h for more info.

    * Fix an issue with scale values of zero when passed to the matrix
      converter.

    * Added indentation to various parts of FBX ASCII files.
    
    * Fixed an issue with some curves evaluating to infinite values.
    
    * Multiple blend shapes with the same target resulted in corrupted file,
      this has been corrected.
      
    * An optimization to the file i/o reader and writer improved import
      and export speed up to 60% faster.
      
    * Fixed an issue with constant key mode in the filters.
    
    * Changed the marker look property enum initialization to match the enum
      in the header file.
      
    * Fixed an issue in the classic evaluator which caused channels not
      animated to sometimes return twice the value.

2012.0

    * The FBX SDK Packaging on the Microsoft Windows platform has changed to
      reduce the size of the downloads. Now the different compilers flavors are
      distributed in separate installers. As a result of this, we also
      uniformized the library file names across platforms.

    * The file format version of FBX files as been incremented to 7.2.
      There have been various small changes in the file format to support new
      data.

    * The preprocessor identifier that must be defined when linking with the
      dynamic library version of the FBX SDK has changed from KFBX_DLLINFO to
      FBXSDK_SHARED. KFBX_DLLINFO now serves no purpose. For any other FBX
      SDK configuration no other preprocessor identifiers must be defined.
      
         Note: Please change your projects to define FBXSDK_SHARED if you
         intend to use the dynamic library version of the FBX SDK. 

    * Many improvements to the KFbxCharacter class have been made to improve 
      interoperability of character data with other Autodesk 3D applications.

    * Support for vector displacement maps has been added to the FBX SDK.

    * Support for Allegorithmic's Substance (procedural textures) has been
      added to the FBX SDK core. Now the class KFbxTexture serves as the base
      class for KFbxFileTexture, KFbxLayeredTexture and KFbxProceduralTexture.
      KFbxFileTexture should now be used for bitmap textures, replacing
      KFbxTexture.

    * The Collada importer now supports the Collada 1.4.1 standard.

    * New quaternion evaluation modes for rotation animation curves are now
      available, equivalent to those found in Autodesk Maya.

    * A new type of animation curve key has been added to the FBX SDK core to
      allow for a better support of Autodesk 3dsMax "auto" keys with crease
      in/out.

    * Added support for Dual Quaternion Skinning. Please refer to the new class
      KFbxDualQuaternion for more information.

    * The visibility inheritance behavior of scene nodes have changed to better
      align with popular Autodesk 3D applications.

    * Many improvements to the import time of FBX files have been made.
    
    * The memory footprint of FBX files has been reduced.

    * Support for "in-between" blend shapes has been added in the FBX SDK core
      to allow application plug-ins to import/export these states.

    * Added five new functions to the KFbxNode class :
    
         KFbxXMatrix& KFbxNode::EvaluateGlobalTransform(...)
         KFbxXMatrix& KFbxNode::EvaluateLocalTransform(...)
         KFbxVector4& KFbxNode::EvaluateLocalTranslation(...)
         KFbxVector4& KFbxNode::EvaluateLocalRotation(...)
         KFbxVector4& KFbxNode::EvaluateLocalScaling(...)
      
      These functions are equivalent to calling KFbxScene->
      GetEvaluator()->..., but have been made directly on the FbxNode for
      convenience.
      
    * Added a new function to the KFbxEvaluator abstract class :
    
         void KFbxEvaluator::ResetEvaluationState()
      
      This function enables users to completely clear the whole evaluation
      cache by deleting all entries in it.
      
    * The API documentation generated by Doxygen requires less disk space. Many
      larger images have been removed to make the documentation easier to use.
      
    * Added support for line geometry to FBX via the KFbxLine class.
    
    * Added an option to KMemoryPool to control concurrent allocations.
      Previously, the allocation was always in concurrent mode.
      
    * The FBX SDK is now built with full optimization settings (i.e. both
      size and speed) instead of being optimized for speed only.
      
    * Roughly 25 new texture blend modes were added. Also, the default blend
      mode is now Normal instead of Translucent.
      
    * Object names will now appear in ASCII FBX files as comments in the
      connection section for easier debugging.
      
    * A new tessellation function was added to NURB curves to provide a work-
      around for applications that do not support them. The tessellation will
      produce a KFbxLine object (a series of vertices).
      
    * A new file, fbxfilesdk_version.h has been added that contains all the
      versioning information of the FBX SDK. It also defines a new preprocessor
      identifier FBXSDK_VERSION_STRING that represents the version information
      as a string.
      
    * Ptex files can be transported just like any other texture format (png,
      jpeg, etc.) provided that they are connected to a material property. For
      more information about Ptex please visit http://ptex.us/documentation.html
      
    * A new function KFbxImporter::SetEmbeddingExtractionFolder as been added
      to allow developers to specify an extract folder when importing FBX files.
      
    * New functions added to KFbxCache class: GetNextTimeWithData(),
      GetDataCount() and GetDataTime().
      
    * All FBX related DLLs shipped are now officially signed with the Autodesk
      certificate.
     
    * The two KFbxSdkManager::CreateClass(...) functions have been marked
      deprecated and KFbxSdkManager::CreateNewObjectFromClassId(...) should be
      used instead.
      
    * The methods KFbxNode::Set/GetUseQuaternionForInterpolation() have been
      marked deprecated. Their usage should be replaced with calls to
      KFbxNode::Set/GetQuaternionInterpolation() respectively. These new
      methods are using the following enumeration symbols to activate new
      evaluation algorithms:

         eQUATINTERP_OFF = 0
         eQUATINTERP_CLASSIC = 1
         eQUATINTERP_SLERP
         eQUATINTERP_CUBIC
         eQUATINTERP_TANGENTDEPENDENT

      The eQUATINTERP_OFF is equivalent to the old way of passing the "false"
      value to the parameter of the SetUseQuaternionForInterpolation() while
      eQUATINTERP_CLASSIC correspond to the "true" value. The classic mode call
      the evaluation algorithm that has been defined in the FBX SDK from the
      earlier versions.
      
    * The methods KFbxSdkManager::CreateClass have been deprecated in favor for
      a new function, KFbxSdkManager::CreateNewObjectFromClassId(...) which
      removes non-used parameters, and combine all functions into this single
      call. The new function name also clearly indicate that a new object
      instance is created, rather than a new class.
      
    * The methods to access the properties of KFbxSurfaceMaterial and sub-class
      have been deprecated. Now users can simply access the properties directly
      just like any other FBX SDK object class.
      
    * Many functions of KFbxShape have been deprecated with the introduction
      of the support for in-between blendshapes.
      
    * The .3DS file format writing as been retired. However, the .3DS file
      reader will persist.

    * The mDEBUG_String member was removed from the KString class to make sure
      the debug and release version are the same size. As a result of this, to
      debug the KString class on the Microsoft Windows platform, please open
      and read the file kstring.txt provided along this readme.txt file.

    * The static libraries are now built with _SECURE_SCL=1, which is the
      default value when not specified by projects. If your project used to
      define _SECURE_SCL=0 just to be able to link with the FBX SDK, then it
      can now be safely removed from the predefined preprocessor identifiers.

    * KFbxObject::Clone has been reviewed, and now offers an implementation
      that should fit most, if not all, cloning logic for all classes that
      inherit from KFbxObject. As a result objects that don't implement Clone
      behave correctly and similarly across the SDK.
      
    * Fixed an issue in the conversion of Euler value to quaternion values with 
      some corner cases.
      
    * The FBX SDK no longer requires RTTI to be enabled by the host software.
      This also means dynamic_cast can no longer be used to convert FBX SDK
      objects. KFbxCast<CLASSTYPE> should be used instead.
      
    * The class KFbxSurfacePhong can now be converted into its parent class,
      KFbxSurfaceLambert, using KFbxCast.

    * The properties of KFbxSurfaceMaterial and all its sub-classes are now
      declared public.
      
    * The function KFbxObject::GetTypeName() has been re-vamped to return a
      const char* type instead of KString. Many classes that were
      re-implementing this function without reason have been cleaned-up
      throughout the SDK.
      
    * The function ResetPivotSetAndConvertAnimation will now also iterate
      through all animation stacks instead of just the first one it finds in
      the scene.
      
    * Fixed a crash wih KFbxAnimEvaluator::SetContext(...).
    
    * A pragma pack directive was added to the file fbxsdk.h to set it to the
      value used when the FBX SDK was compiled. This will allow developers
      to include the FBX SDK in their project without having to worry about the
      packing size used by their application.
      
    * KFbxCache::GetChannelIndex() should return -1 instead of 0 when error
      occurs or channel not found.
      
    * Fixed several issues and memory leaks in FBX SDK sample codes.
    
    * KString size in debug and release should now be the same, which fix the
      crash resulting of that incompatibility.
      
    * The Microsoft Visual Studio 2008 builds are now compiled using the
      Service Pack 1.
      
    * Added the const qualifier to several functions and members.
    
    * Stereo Cameras now have the correct post-rotation set on export.
    
    * An issue with multiple property-to-property connections in FBX files has
      been corrected.
      
    * An optimization to the way we allocate animation curves has been done to
      improve the destroy speed with large scenes.
      
    * Fixed an issue in KFbxMesh::TriangulateMeshAdvance(), an incorrect mesh
      smooth group was set when the original mapping mode was "by edge".
      
    * An issue with skew matrix in the classic evaluator has been fixed.

2011.3.1

    * All functions related to local and global "state" in the KFbxNode
      declaration were removed since their implementation didn't work anymore
      since the last release. They should have been set to deprecated within
      the header files, but they were forgotten.
      
    * The unroll filter was inconsistent when applying its process on curves
      data, this has been corrected.

    * Fixed a crash in the function EmulateNormalsByPolygonVertex.
    
    * Visual Studio 2010 builds were reporting missing PDB files, this has been
      corrected.
      
    * Importing various .OBJ file sometime resulted in much more vertices, this
      is now working as intended.
      
    * The mesh triangulation method has been re-written for a much more robust
      algorithm. Now it can handle non-concave shapes. The method can be called
      using TriangulateMeshAdvanced.
      
    * A conversion error from Euler to Quaternion has been corrected, it is now
      much more robust for corner cases.

2011.3

    * The FBX SDK is now also ship in a dynamic library package format. To link
      against the dynamic version, users will have to define KFBX_DLLINFO in
      their project preprocessor settings. The static version still doesn't
      require any preprocessor to be defined.

    * Augmented the KFbxCache class to correctly support multiple data channels.

    * Stereo Cameras now correctly support aim/up.
    
    * Added three new functions to the KFbxAnimEvaluator class :
    
      KFbxVector4& GetNodeLocalTranslation(...)
      KFbxVector4& GetNodeLocalRotation(...)
      KFbxVector4& GetNodeLocalScaling(...)
      
      Allow users to calculate the local translation, rotation and scaling as it
      was done in previous versions of the FBX SDK. On purpose, these new
      functions will not take pre/post rotations, offsets and pivots in
      consideration, but they still will consider translation, rotation and
      scaling limits.

    * Added the new KFbxCachedEffect node attribute. These are used to store
      other kind of vertex (or position) caching.

    * Fixed a crash at import time for various legacy FBX files when importing
      animation curves data.

    * Some UV Sets were lost in very specific cases during export, this has
      been corrected.

    * Fixed an issue with node's local transform calculation. Now it should
      correctly return the result of ParentGlobal.Inverse * Global.

    * Protein 2.0 Materials are now extract in the .fbm folder along the .fbx
      file first, rather than the user's operating system temporary folder.
      
    * The following files contain many newly deprecated calls. Please open them
      and search for the macro K_DEPRECATED to find out. Because the list is so
      big, it will not be listed here.
      
      kfbxconstraint.h, kfbxconstraintaim.h, kfbxconstraintparent.h,
      kfbxgeometry.h, kfbxnode.h, kfbxscene.h, kfbxkfcurvefilters.h,
      kfbxdocument.h, kfbxreaderfbx.h, kfbxreaderfbx6.h, kfbximporter.h,
      kfbxproperty.h

2011.2

    * Officially dropped support for PowerPC architecture. Universal binaries
      found in MacOS builds will now only contain 32/64-bit variances.

    * Fixed a crash when importing some legacy MotionBuilder FBX 5.x files.
    
    * Corrected the computation of the smoothing group for in mesh triangulation
      function.
    
    * Fixed Localization problems on the DAE, DFX and OBJ readers/writers.
    
    * Extended the file size limit for FBX files from 2GB to 4GB.
    
    * Augmented the Reference Documentation for a certain number of classes. For
      example, check out KFbxNode or KFbxObject and tell us what you think! :)

2011.1

    * Removed the KFbxTakeNodeContainer class. This was done with the redesign
      of the animation system.

    * A whole new set of classes are now available to evaluate animation with
      the FBX SDK. For more information, please look at the reference
      documentation for KFbxAnimStack, KFbxAnimLayer, KFbxAnimCurve,
      KFbxAnimCurveNode, KFbxAnimEvaluator and KFbxAnimEvalClassic classes. Also
      the evaluation result will now be stored outside the KFbxNode class and
      only created on demand, resulting in a much smaller memory footprint.

    * Removed all needed preprocessor defines to be able to correctly link with
      the static version of the FBX SDK. Namely, those defines were K_PLUGIN,
      K_FBXSDK and K_NODLL.
      
    * The FBX file format as now been upgraded to our latest new technology,
      FBX 7.1! This new FBX file format allow for much more flexibility,
      supporting any number of instances, connections by GUID, reduced file size
      with compression, embedding in ASCII files and much more!
      
    * The KFbxSystemUnit class changed so that it doesn't modify the multiplier
      parameter. Now it is simply carried along in the FBX file.
      
    * A Python Binding for FBX SDK has been released for the first time! In this
      first release, only the most basic functions to allow import/export and
      scene traversal and property query has been exposed. More will be exposed
      later on when we gather more feedback from user experience.
      
2010.2

    * Improved processing speed of the function to retrieve polygon's indexes in
      the mesh class KFbxMesh.
      
    * Removed SetFileFormat on all classes that inherit from KFbxImporter and
      KFbxExporter. Instead, the file format can be overwritten in the
      Initialize functions. By default, the file format will now be auto-
      matically be detected.
      
    * Extended the KFbxMesh class to support standard mesh smoothing. We are
      referring to edge/vertex creases, mesh smoothness, division levels,
      subdivisions, continuity and border/edge preservation.

    * Added Stereo Cameras support via the KFbxCameraStereo class.

    * Added Display Layer and Selection Sets support via the KFbxDisplayLayer
      and KFbxSelectionSet classes respectively.

    * Fixed an issue preventing the use of the slash ( / ) character in property
      names.
      
    * Fixed a stack overflow error in KFbxRenamingStrategy.
    
    * Added support for many new texture blend mode. See KFbxLayerElementTexture
      for more information.

    * Files not ending in .fbx but that still contain FBX formatting can now
      successfully be opened with the FBX SDK.
      
    * Properties can now be destroyed with KFbxProperty::Destroy().

    * Fixed FBX 5.x reader to correctly set pivot information when importing
      legacy files.
      
2010.0

    * Dropped support for Microsoft Visual Studio 2003 libraries.
    
    * Many, many issues fixed. Please refer to previous readme versions for more
      details.
      
2009.x

    * KFbxCache class supports int array.
    
    * Added the Subdivision Display Smoothness to the KFbxSubdiv class.
    
    * Added the optional argument to the IsValid() method in the KFbxTrimSurface
      class to skip the check of boundary curves CV's overlaps.
      
    * Re-factoring of the KFbxCamera class.
    
    * Updates to class documentation.

    * Added methods and properties to manipulate front/back planes & plates.
    
    * Deprecated ECameraBackgroundDrawingMode type and replaced with
      ECameraPlateDrawingMode
    
    * Deprecated ECameraBackgroundPlacementOptions. This has been replaced with
      the individual properties: FitImage, Center, KeepRatio and Crop.
    
    * Deprecated GetBackgroundPlacementOptions() since now the flags are stored
      in the above mentioned properties.
    
    * Deprecated SetViewFrustum(bool pEnable), use SetViewNearFarPlanes()
    
    * Deprecated GetViewFrustum(), use GetViewNearFarPlanes()
    
    * Support of non-convex polygons in the triangulation algorithms.
    
    * Overload of the new operator is not possible anymore (the
      FBXSDK_OVERLOAD_NEW_OPERATOR macro has been removed). The usage of the
      custom memory allocator can only be achieve by using the
      KFbxMemoryAllocator class. See the ExportScene05 for an implementation
      example.
    
    * Enhanced algorithm for smoothing groups generation.
    
    * Support of displacement map channel.
    
    * The class KFbxStreamOptions is now obsolete and is gradually being
      replaced by the class KFbxIOSettings.
    
    * Added KFbxConstraintCustom class.
    
    * Added KFbxContainerTemplate class.
    
    * Added KFbxSubdiv class.
    
    * Added KFbxEmbeddedFilesAccumulator class.
    
    * Adjusted tangents to stay closer to the real value when the weight gets
      ridiculously small.
    
    * Fixed Collada plug-in to handle operating system locale. Depending on the
      locale, the decimal point for numbers may have been represented with the
      comma instead of the point causing parsing errors.
      
    * Fixed support for the floor contact to the KFbxCharacter.
    
    * Fixed infinite loop when loading .obj files on MAC OS.
    
    * Removed some more memory leaks.
    
    * Added the HasDefaultValue(KFbxProperty&) function to check if a property
      value has changed from its default one.
    
    * Added the BumpFactor property to the SurfaceMaterial class.
    
    * Defined plug-ins of plug-ins interface in the fbxsdk manager. See the
      Autodesk FBX SDK PRW readme file for more details.
    
    * Re-factoring of the renaming strategy object.
    
    * Removed unused eCONSTRAINT from the KFbxNodeAttribute.
    
    * Deprecated FillNodeArray and FillNodeArrayRecursive
    
    * Overwrite empty relative filename in texture objects with the correct
      value.
    
    * Fix for internal TRS cache so it correctly get reset when changing takes.
    
    * Bug fixes in the Collada reader/writer.
    
    * Fixed a bug that was causing the loss of Shape animation on NURBS objects.
    
    * Fixed the fact that the layers were losing their name after a clone.
    
    * Corrections for pivot conversion functions:
    
      - Set source pivot to ACTIVE in function ResetPivotSetAndConvertAnimation.
      
      - Update default transformation values to match the results of the pivot
        conversion functions.
      
    * Fixed the endless loop in the RemoveChar() method of the KString class.
    
    * Fixed default values in the KFbxCharacter structure.



4. LEGAL DISCLAIMER
-------------------

Autodesk and FBX are registered trademarks or trademarks of Autodesk, Inc., in
the USA and/or other countries. All other brand names, product names, or trade-
marks belong to their respective holders.

                       Copyright (C) 2018 Autodesk, Inc.
                              All Rights Reserved

================================================================================
