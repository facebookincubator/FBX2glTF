# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /document/FBX2glTF/build/draco/src/Draco

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /document/FBX2glTF/build/draco/src/Draco-build

# Include any dependencies generated for this target.
include CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/flags.make

CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/draco_compression_attributes_pred_schemes_dec.cc.o: CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/flags.make
CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/draco_compression_attributes_pred_schemes_dec.cc.o: draco_compression_attributes_pred_schemes_dec.cc
CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/draco_compression_attributes_pred_schemes_dec.cc.o: CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/document/FBX2glTF/build/draco/src/Draco-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/draco_compression_attributes_pred_schemes_dec.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/draco_compression_attributes_pred_schemes_dec.cc.o -MF CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/draco_compression_attributes_pred_schemes_dec.cc.o.d -o CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/draco_compression_attributes_pred_schemes_dec.cc.o -c /document/FBX2glTF/build/draco/src/Draco-build/draco_compression_attributes_pred_schemes_dec.cc

CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/draco_compression_attributes_pred_schemes_dec.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/draco_compression_attributes_pred_schemes_dec.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /document/FBX2glTF/build/draco/src/Draco-build/draco_compression_attributes_pred_schemes_dec.cc > CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/draco_compression_attributes_pred_schemes_dec.cc.i

CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/draco_compression_attributes_pred_schemes_dec.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/draco_compression_attributes_pred_schemes_dec.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /document/FBX2glTF/build/draco/src/Draco-build/draco_compression_attributes_pred_schemes_dec.cc -o CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/draco_compression_attributes_pred_schemes_dec.cc.s

draco_compression_attributes_pred_schemes_dec: CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/draco_compression_attributes_pred_schemes_dec.cc.o
draco_compression_attributes_pred_schemes_dec: CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/build.make
.PHONY : draco_compression_attributes_pred_schemes_dec

# Rule to build all files generated by this target.
CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/build: draco_compression_attributes_pred_schemes_dec
.PHONY : CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/build

CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/cmake_clean.cmake
.PHONY : CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/clean

CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/depend:
	cd /document/FBX2glTF/build/draco/src/Draco-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /document/FBX2glTF/build/draco/src/Draco /document/FBX2glTF/build/draco/src/Draco /document/FBX2glTF/build/draco/src/Draco-build /document/FBX2glTF/build/draco/src/Draco-build /document/FBX2glTF/build/draco/src/Draco-build/CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/draco_compression_attributes_pred_schemes_dec.dir/depend
