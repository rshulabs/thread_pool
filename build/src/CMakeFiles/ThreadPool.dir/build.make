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
CMAKE_SOURCE_DIR = /root/workspace/cpp/线程池项目

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/workspace/cpp/线程池项目/build

# Include any dependencies generated for this target.
include src/CMakeFiles/ThreadPool.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/CMakeFiles/ThreadPool.dir/compiler_depend.make

# Include the progress variables for this target.
include src/CMakeFiles/ThreadPool.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/ThreadPool.dir/flags.make

src/CMakeFiles/ThreadPool.dir/main.cpp.o: src/CMakeFiles/ThreadPool.dir/flags.make
src/CMakeFiles/ThreadPool.dir/main.cpp.o: ../src/main.cpp
src/CMakeFiles/ThreadPool.dir/main.cpp.o: src/CMakeFiles/ThreadPool.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/workspace/cpp/线程池项目/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/CMakeFiles/ThreadPool.dir/main.cpp.o"
	cd /root/workspace/cpp/线程池项目/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/ThreadPool.dir/main.cpp.o -MF CMakeFiles/ThreadPool.dir/main.cpp.o.d -o CMakeFiles/ThreadPool.dir/main.cpp.o -c /root/workspace/cpp/线程池项目/src/main.cpp

src/CMakeFiles/ThreadPool.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ThreadPool.dir/main.cpp.i"
	cd /root/workspace/cpp/线程池项目/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/workspace/cpp/线程池项目/src/main.cpp > CMakeFiles/ThreadPool.dir/main.cpp.i

src/CMakeFiles/ThreadPool.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ThreadPool.dir/main.cpp.s"
	cd /root/workspace/cpp/线程池项目/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/workspace/cpp/线程池项目/src/main.cpp -o CMakeFiles/ThreadPool.dir/main.cpp.s

src/CMakeFiles/ThreadPool.dir/thread_pool.cpp.o: src/CMakeFiles/ThreadPool.dir/flags.make
src/CMakeFiles/ThreadPool.dir/thread_pool.cpp.o: ../src/thread_pool.cpp
src/CMakeFiles/ThreadPool.dir/thread_pool.cpp.o: src/CMakeFiles/ThreadPool.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/workspace/cpp/线程池项目/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/CMakeFiles/ThreadPool.dir/thread_pool.cpp.o"
	cd /root/workspace/cpp/线程池项目/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/ThreadPool.dir/thread_pool.cpp.o -MF CMakeFiles/ThreadPool.dir/thread_pool.cpp.o.d -o CMakeFiles/ThreadPool.dir/thread_pool.cpp.o -c /root/workspace/cpp/线程池项目/src/thread_pool.cpp

src/CMakeFiles/ThreadPool.dir/thread_pool.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ThreadPool.dir/thread_pool.cpp.i"
	cd /root/workspace/cpp/线程池项目/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/workspace/cpp/线程池项目/src/thread_pool.cpp > CMakeFiles/ThreadPool.dir/thread_pool.cpp.i

src/CMakeFiles/ThreadPool.dir/thread_pool.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ThreadPool.dir/thread_pool.cpp.s"
	cd /root/workspace/cpp/线程池项目/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/workspace/cpp/线程池项目/src/thread_pool.cpp -o CMakeFiles/ThreadPool.dir/thread_pool.cpp.s

# Object files for target ThreadPool
ThreadPool_OBJECTS = \
"CMakeFiles/ThreadPool.dir/main.cpp.o" \
"CMakeFiles/ThreadPool.dir/thread_pool.cpp.o"

# External object files for target ThreadPool
ThreadPool_EXTERNAL_OBJECTS =

../bin/ThreadPool: src/CMakeFiles/ThreadPool.dir/main.cpp.o
../bin/ThreadPool: src/CMakeFiles/ThreadPool.dir/thread_pool.cpp.o
../bin/ThreadPool: src/CMakeFiles/ThreadPool.dir/build.make
../bin/ThreadPool: src/CMakeFiles/ThreadPool.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/root/workspace/cpp/线程池项目/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable ../../bin/ThreadPool"
	cd /root/workspace/cpp/线程池项目/build/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ThreadPool.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/ThreadPool.dir/build: ../bin/ThreadPool
.PHONY : src/CMakeFiles/ThreadPool.dir/build

src/CMakeFiles/ThreadPool.dir/clean:
	cd /root/workspace/cpp/线程池项目/build/src && $(CMAKE_COMMAND) -P CMakeFiles/ThreadPool.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/ThreadPool.dir/clean

src/CMakeFiles/ThreadPool.dir/depend:
	cd /root/workspace/cpp/线程池项目/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/workspace/cpp/线程池项目 /root/workspace/cpp/线程池项目/src /root/workspace/cpp/线程池项目/build /root/workspace/cpp/线程池项目/build/src /root/workspace/cpp/线程池项目/build/src/CMakeFiles/ThreadPool.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/ThreadPool.dir/depend
