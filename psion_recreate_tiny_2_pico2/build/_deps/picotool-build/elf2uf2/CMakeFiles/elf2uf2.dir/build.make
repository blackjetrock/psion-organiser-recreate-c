# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.31

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
CMAKE_SOURCE_DIR = /home/menadue/tree/github/psion-organiser-recreate-c/psion_recreate_tiny_2_pico2/build/_deps/picotool-src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/menadue/tree/github/psion-organiser-recreate-c/psion_recreate_tiny_2_pico2/build/_deps/picotool-build

# Include any dependencies generated for this target.
include elf2uf2/CMakeFiles/elf2uf2.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include elf2uf2/CMakeFiles/elf2uf2.dir/compiler_depend.make

# Include the compile flags for this target's objects.
include elf2uf2/CMakeFiles/elf2uf2.dir/flags.make

elf2uf2/CMakeFiles/elf2uf2.dir/codegen:
.PHONY : elf2uf2/CMakeFiles/elf2uf2.dir/codegen

elf2uf2/CMakeFiles/elf2uf2.dir/elf2uf2.cpp.o: elf2uf2/CMakeFiles/elf2uf2.dir/flags.make
elf2uf2/CMakeFiles/elf2uf2.dir/elf2uf2.cpp.o: /home/menadue/tree/github/psion-organiser-recreate-c/psion_recreate_tiny_2_pico2/build/_deps/picotool-src/elf2uf2/elf2uf2.cpp
elf2uf2/CMakeFiles/elf2uf2.dir/elf2uf2.cpp.o: elf2uf2/CMakeFiles/elf2uf2.dir/compiler_depend.ts
	cd /home/menadue/tree/github/psion-organiser-recreate-c/psion_recreate_tiny_2_pico2/build/_deps/picotool-build/elf2uf2 && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT elf2uf2/CMakeFiles/elf2uf2.dir/elf2uf2.cpp.o -MF CMakeFiles/elf2uf2.dir/elf2uf2.cpp.o.d -o CMakeFiles/elf2uf2.dir/elf2uf2.cpp.o -c /home/menadue/tree/github/psion-organiser-recreate-c/psion_recreate_tiny_2_pico2/build/_deps/picotool-src/elf2uf2/elf2uf2.cpp

elf2uf2/CMakeFiles/elf2uf2.dir/elf2uf2.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/elf2uf2.dir/elf2uf2.cpp.i"
	cd /home/menadue/tree/github/psion-organiser-recreate-c/psion_recreate_tiny_2_pico2/build/_deps/picotool-build/elf2uf2 && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/menadue/tree/github/psion-organiser-recreate-c/psion_recreate_tiny_2_pico2/build/_deps/picotool-src/elf2uf2/elf2uf2.cpp > CMakeFiles/elf2uf2.dir/elf2uf2.cpp.i

elf2uf2/CMakeFiles/elf2uf2.dir/elf2uf2.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/elf2uf2.dir/elf2uf2.cpp.s"
	cd /home/menadue/tree/github/psion-organiser-recreate-c/psion_recreate_tiny_2_pico2/build/_deps/picotool-build/elf2uf2 && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/menadue/tree/github/psion-organiser-recreate-c/psion_recreate_tiny_2_pico2/build/_deps/picotool-src/elf2uf2/elf2uf2.cpp -o CMakeFiles/elf2uf2.dir/elf2uf2.cpp.s

# Object files for target elf2uf2
elf2uf2_OBJECTS = \
"CMakeFiles/elf2uf2.dir/elf2uf2.cpp.o"

# External object files for target elf2uf2
elf2uf2_EXTERNAL_OBJECTS =

elf2uf2/libelf2uf2.a: elf2uf2/CMakeFiles/elf2uf2.dir/elf2uf2.cpp.o
elf2uf2/libelf2uf2.a: elf2uf2/CMakeFiles/elf2uf2.dir/build.make
elf2uf2/libelf2uf2.a: elf2uf2/CMakeFiles/elf2uf2.dir/link.txt
	cd /home/menadue/tree/github/psion-organiser-recreate-c/psion_recreate_tiny_2_pico2/build/_deps/picotool-build/elf2uf2 && $(CMAKE_COMMAND) -P CMakeFiles/elf2uf2.dir/cmake_clean_target.cmake
	cd /home/menadue/tree/github/psion-organiser-recreate-c/psion_recreate_tiny_2_pico2/build/_deps/picotool-build/elf2uf2 && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/elf2uf2.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
elf2uf2/CMakeFiles/elf2uf2.dir/build: elf2uf2/libelf2uf2.a
.PHONY : elf2uf2/CMakeFiles/elf2uf2.dir/build

elf2uf2/CMakeFiles/elf2uf2.dir/clean:
	cd /home/menadue/tree/github/psion-organiser-recreate-c/psion_recreate_tiny_2_pico2/build/_deps/picotool-build/elf2uf2 && $(CMAKE_COMMAND) -P CMakeFiles/elf2uf2.dir/cmake_clean.cmake
.PHONY : elf2uf2/CMakeFiles/elf2uf2.dir/clean

elf2uf2/CMakeFiles/elf2uf2.dir/depend:
	cd /home/menadue/tree/github/psion-organiser-recreate-c/psion_recreate_tiny_2_pico2/build/_deps/picotool-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/menadue/tree/github/psion-organiser-recreate-c/psion_recreate_tiny_2_pico2/build/_deps/picotool-src /home/menadue/tree/github/psion-organiser-recreate-c/psion_recreate_tiny_2_pico2/build/_deps/picotool-src/elf2uf2 /home/menadue/tree/github/psion-organiser-recreate-c/psion_recreate_tiny_2_pico2/build/_deps/picotool-build /home/menadue/tree/github/psion-organiser-recreate-c/psion_recreate_tiny_2_pico2/build/_deps/picotool-build/elf2uf2 /home/menadue/tree/github/psion-organiser-recreate-c/psion_recreate_tiny_2_pico2/build/_deps/picotool-build/elf2uf2/CMakeFiles/elf2uf2.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : elf2uf2/CMakeFiles/elf2uf2.dir/depend

