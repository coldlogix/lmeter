# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.6

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.6.1/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.6.1/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/travis/build/coldlogix/lmeter

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/travis/build/coldlogix/lmeter/build

# Include any dependencies generated for this target.
include contrib/CMakeFiles/ltm.dir/depend.make

# Include the progress variables for this target.
include contrib/CMakeFiles/ltm.dir/progress.make

# Include the compile flags for this target's objects.
include contrib/CMakeFiles/ltm.dir/flags.make

contrib/CMakeFiles/ltm.dir/ltm1.cpp.o: contrib/CMakeFiles/ltm.dir/flags.make
contrib/CMakeFiles/ltm.dir/ltm1.cpp.o: ../contrib/ltm1.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/travis/build/coldlogix/lmeter/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object contrib/CMakeFiles/ltm.dir/ltm1.cpp.o"
	cd /Users/travis/build/coldlogix/lmeter/build/contrib && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ltm.dir/ltm1.cpp.o -c /Users/travis/build/coldlogix/lmeter/contrib/ltm1.cpp

contrib/CMakeFiles/ltm.dir/ltm1.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ltm.dir/ltm1.cpp.i"
	cd /Users/travis/build/coldlogix/lmeter/build/contrib && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/travis/build/coldlogix/lmeter/contrib/ltm1.cpp > CMakeFiles/ltm.dir/ltm1.cpp.i

contrib/CMakeFiles/ltm.dir/ltm1.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ltm.dir/ltm1.cpp.s"
	cd /Users/travis/build/coldlogix/lmeter/build/contrib && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/travis/build/coldlogix/lmeter/contrib/ltm1.cpp -o CMakeFiles/ltm.dir/ltm1.cpp.s

contrib/CMakeFiles/ltm.dir/ltm1.cpp.o.requires:

.PHONY : contrib/CMakeFiles/ltm.dir/ltm1.cpp.o.requires

contrib/CMakeFiles/ltm.dir/ltm1.cpp.o.provides: contrib/CMakeFiles/ltm.dir/ltm1.cpp.o.requires
	$(MAKE) -f contrib/CMakeFiles/ltm.dir/build.make contrib/CMakeFiles/ltm.dir/ltm1.cpp.o.provides.build
.PHONY : contrib/CMakeFiles/ltm.dir/ltm1.cpp.o.provides

contrib/CMakeFiles/ltm.dir/ltm1.cpp.o.provides.build: contrib/CMakeFiles/ltm.dir/ltm1.cpp.o


# Object files for target ltm
ltm_OBJECTS = \
"CMakeFiles/ltm.dir/ltm1.cpp.o"

# External object files for target ltm
ltm_EXTERNAL_OBJECTS =

contrib/ltm: contrib/CMakeFiles/ltm.dir/ltm1.cpp.o
contrib/ltm: contrib/CMakeFiles/ltm.dir/build.make
contrib/ltm: contrib/CMakeFiles/ltm.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/travis/build/coldlogix/lmeter/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ltm"
	cd /Users/travis/build/coldlogix/lmeter/build/contrib && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ltm.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
contrib/CMakeFiles/ltm.dir/build: contrib/ltm

.PHONY : contrib/CMakeFiles/ltm.dir/build

contrib/CMakeFiles/ltm.dir/requires: contrib/CMakeFiles/ltm.dir/ltm1.cpp.o.requires

.PHONY : contrib/CMakeFiles/ltm.dir/requires

contrib/CMakeFiles/ltm.dir/clean:
	cd /Users/travis/build/coldlogix/lmeter/build/contrib && $(CMAKE_COMMAND) -P CMakeFiles/ltm.dir/cmake_clean.cmake
.PHONY : contrib/CMakeFiles/ltm.dir/clean

contrib/CMakeFiles/ltm.dir/depend:
	cd /Users/travis/build/coldlogix/lmeter/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/travis/build/coldlogix/lmeter /Users/travis/build/coldlogix/lmeter/contrib /Users/travis/build/coldlogix/lmeter/build /Users/travis/build/coldlogix/lmeter/build/contrib /Users/travis/build/coldlogix/lmeter/build/contrib/CMakeFiles/ltm.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : contrib/CMakeFiles/ltm.dir/depend
