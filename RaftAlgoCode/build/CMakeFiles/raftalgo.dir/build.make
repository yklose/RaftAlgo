# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /root/Beaglebone/PPL_git/RaftAlgo/RaftAlgoCode

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/Beaglebone/PPL_git/RaftAlgo/RaftAlgoCode/build

# Include any dependencies generated for this target.
include CMakeFiles/raftalgo.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/raftalgo.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/raftalgo.dir/flags.make

CMakeFiles/raftalgo.dir/src/lib.c.o: CMakeFiles/raftalgo.dir/flags.make
CMakeFiles/raftalgo.dir/src/lib.c.o: ../src/lib.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/Beaglebone/PPL_git/RaftAlgo/RaftAlgoCode/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/raftalgo.dir/src/lib.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/raftalgo.dir/src/lib.c.o   -c /root/Beaglebone/PPL_git/RaftAlgo/RaftAlgoCode/src/lib.c

CMakeFiles/raftalgo.dir/src/lib.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/raftalgo.dir/src/lib.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /root/Beaglebone/PPL_git/RaftAlgo/RaftAlgoCode/src/lib.c > CMakeFiles/raftalgo.dir/src/lib.c.i

CMakeFiles/raftalgo.dir/src/lib.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/raftalgo.dir/src/lib.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /root/Beaglebone/PPL_git/RaftAlgo/RaftAlgoCode/src/lib.c -o CMakeFiles/raftalgo.dir/src/lib.c.s

CMakeFiles/raftalgo.dir/src/lib.c.o.requires:

.PHONY : CMakeFiles/raftalgo.dir/src/lib.c.o.requires

CMakeFiles/raftalgo.dir/src/lib.c.o.provides: CMakeFiles/raftalgo.dir/src/lib.c.o.requires
	$(MAKE) -f CMakeFiles/raftalgo.dir/build.make CMakeFiles/raftalgo.dir/src/lib.c.o.provides.build
.PHONY : CMakeFiles/raftalgo.dir/src/lib.c.o.provides

CMakeFiles/raftalgo.dir/src/lib.c.o.provides.build: CMakeFiles/raftalgo.dir/src/lib.c.o


CMakeFiles/raftalgo.dir/src/raftalgo.c.o: CMakeFiles/raftalgo.dir/flags.make
CMakeFiles/raftalgo.dir/src/raftalgo.c.o: ../src/raftalgo.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/Beaglebone/PPL_git/RaftAlgo/RaftAlgoCode/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/raftalgo.dir/src/raftalgo.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/raftalgo.dir/src/raftalgo.c.o   -c /root/Beaglebone/PPL_git/RaftAlgo/RaftAlgoCode/src/raftalgo.c

CMakeFiles/raftalgo.dir/src/raftalgo.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/raftalgo.dir/src/raftalgo.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /root/Beaglebone/PPL_git/RaftAlgo/RaftAlgoCode/src/raftalgo.c > CMakeFiles/raftalgo.dir/src/raftalgo.c.i

CMakeFiles/raftalgo.dir/src/raftalgo.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/raftalgo.dir/src/raftalgo.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /root/Beaglebone/PPL_git/RaftAlgo/RaftAlgoCode/src/raftalgo.c -o CMakeFiles/raftalgo.dir/src/raftalgo.c.s

CMakeFiles/raftalgo.dir/src/raftalgo.c.o.requires:

.PHONY : CMakeFiles/raftalgo.dir/src/raftalgo.c.o.requires

CMakeFiles/raftalgo.dir/src/raftalgo.c.o.provides: CMakeFiles/raftalgo.dir/src/raftalgo.c.o.requires
	$(MAKE) -f CMakeFiles/raftalgo.dir/build.make CMakeFiles/raftalgo.dir/src/raftalgo.c.o.provides.build
.PHONY : CMakeFiles/raftalgo.dir/src/raftalgo.c.o.provides

CMakeFiles/raftalgo.dir/src/raftalgo.c.o.provides.build: CMakeFiles/raftalgo.dir/src/raftalgo.c.o


# Object files for target raftalgo
raftalgo_OBJECTS = \
"CMakeFiles/raftalgo.dir/src/lib.c.o" \
"CMakeFiles/raftalgo.dir/src/raftalgo.c.o"

# External object files for target raftalgo
raftalgo_EXTERNAL_OBJECTS =

raftalgo: CMakeFiles/raftalgo.dir/src/lib.c.o
raftalgo: CMakeFiles/raftalgo.dir/src/raftalgo.c.o
raftalgo: CMakeFiles/raftalgo.dir/build.make
raftalgo: CMakeFiles/raftalgo.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/root/Beaglebone/PPL_git/RaftAlgo/RaftAlgoCode/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable raftalgo"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/raftalgo.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/raftalgo.dir/build: raftalgo

.PHONY : CMakeFiles/raftalgo.dir/build

CMakeFiles/raftalgo.dir/requires: CMakeFiles/raftalgo.dir/src/lib.c.o.requires
CMakeFiles/raftalgo.dir/requires: CMakeFiles/raftalgo.dir/src/raftalgo.c.o.requires

.PHONY : CMakeFiles/raftalgo.dir/requires

CMakeFiles/raftalgo.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/raftalgo.dir/cmake_clean.cmake
.PHONY : CMakeFiles/raftalgo.dir/clean

CMakeFiles/raftalgo.dir/depend:
	cd /root/Beaglebone/PPL_git/RaftAlgo/RaftAlgoCode/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/Beaglebone/PPL_git/RaftAlgo/RaftAlgoCode /root/Beaglebone/PPL_git/RaftAlgo/RaftAlgoCode /root/Beaglebone/PPL_git/RaftAlgo/RaftAlgoCode/build /root/Beaglebone/PPL_git/RaftAlgo/RaftAlgoCode/build /root/Beaglebone/PPL_git/RaftAlgo/RaftAlgoCode/build/CMakeFiles/raftalgo.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/raftalgo.dir/depend

