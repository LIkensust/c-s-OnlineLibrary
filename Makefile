# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


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
CMAKE_SOURCE_DIR = /home/liyuan/study/c-s_base_OnlineLibrary

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/liyuan/study/c-s_base_OnlineLibrary

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake cache editor..."
	/usr/bin/ccmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/liyuan/study/c-s_base_OnlineLibrary/CMakeFiles /home/liyuan/study/c-s_base_OnlineLibrary/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/liyuan/study/c-s_base_OnlineLibrary/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named CreateJson

# Build rule for target.
CreateJson: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 CreateJson
.PHONY : CreateJson

# fast build rule for target.
CreateJson/fast:
	$(MAKE) -f CMakeFiles/CreateJson.dir/build.make CMakeFiles/CreateJson.dir/build
.PHONY : CreateJson/fast

#=============================================================================
# Target rules for targets named redo

# Build rule for target.
redo: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 redo
.PHONY : redo

# fast build rule for target.
redo/fast:
	$(MAKE) -f CMakeFiles/redo.dir/build.make CMakeFiles/redo.dir/build
.PHONY : redo/fast

#=============================================================================
# Target rules for targets named ser

# Build rule for target.
ser: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 ser
.PHONY : ser

# fast build rule for target.
ser/fast:
	$(MAKE) -f CMakeFiles/ser.dir/build.make CMakeFiles/ser.dir/build
.PHONY : ser/fast

#=============================================================================
# Target rules for targets named cli

# Build rule for target.
cli: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 cli
.PHONY : cli

# fast build rule for target.
cli/fast:
	$(MAKE) -f CMakeFiles/cli.dir/build.make CMakeFiles/cli.dir/build
.PHONY : cli/fast

#=============================================================================
# Target rules for targets named CJsonTool

# Build rule for target.
CJsonTool: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 CJsonTool
.PHONY : CJsonTool

# fast build rule for target.
CJsonTool/fast:
	$(MAKE) -f CJsonObject/CMakeFiles/CJsonTool.dir/build.make CJsonObject/CMakeFiles/CJsonTool.dir/build
.PHONY : CJsonTool/fast

#=============================================================================
# Target rules for targets named demo2

# Build rule for target.
demo2: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 demo2
.PHONY : demo2

# fast build rule for target.
demo2/fast:
	$(MAKE) -f demo/CMakeFiles/demo2.dir/build.make demo/CMakeFiles/demo2.dir/build
.PHONY : demo2/fast

#=============================================================================
# Target rules for targets named demo

# Build rule for target.
demo: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 demo
.PHONY : demo

# fast build rule for target.
demo/fast:
	$(MAKE) -f demo/CMakeFiles/demo.dir/build.make demo/CMakeFiles/demo.dir/build
.PHONY : demo/fast

#=============================================================================
# Target rules for targets named demo1

# Build rule for target.
demo1: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 demo1
.PHONY : demo1

# fast build rule for target.
demo1/fast:
	$(MAKE) -f demo/CMakeFiles/demo1.dir/build.make demo/CMakeFiles/demo1.dir/build
.PHONY : demo1/fast

src/Server.o: src/Server.cc.o

.PHONY : src/Server.o

# target to build an object file
src/Server.cc.o:
	$(MAKE) -f CMakeFiles/redo.dir/build.make CMakeFiles/redo.dir/src/Server.cc.o
.PHONY : src/Server.cc.o

src/Server.i: src/Server.cc.i

.PHONY : src/Server.i

# target to preprocess a source file
src/Server.cc.i:
	$(MAKE) -f CMakeFiles/redo.dir/build.make CMakeFiles/redo.dir/src/Server.cc.i
.PHONY : src/Server.cc.i

src/Server.s: src/Server.cc.s

.PHONY : src/Server.s

# target to generate assembly for a file
src/Server.cc.s:
	$(MAKE) -f CMakeFiles/redo.dir/build.make CMakeFiles/redo.dir/src/Server.cc.s
.PHONY : src/Server.cc.s

src/client.o: src/client.cc.o

.PHONY : src/client.o

# target to build an object file
src/client.cc.o:
	$(MAKE) -f CMakeFiles/cli.dir/build.make CMakeFiles/cli.dir/src/client.cc.o
.PHONY : src/client.cc.o

src/client.i: src/client.cc.i

.PHONY : src/client.i

# target to preprocess a source file
src/client.cc.i:
	$(MAKE) -f CMakeFiles/cli.dir/build.make CMakeFiles/cli.dir/src/client.cc.i
.PHONY : src/client.cc.i

src/client.s: src/client.cc.s

.PHONY : src/client.s

# target to generate assembly for a file
src/client.cc.s:
	$(MAKE) -f CMakeFiles/cli.dir/build.make CMakeFiles/cli.dir/src/client.cc.s
.PHONY : src/client.cc.s

src/make_test_json_file.o: src/make_test_json_file.cc.o

.PHONY : src/make_test_json_file.o

# target to build an object file
src/make_test_json_file.cc.o:
	$(MAKE) -f CMakeFiles/CreateJson.dir/build.make CMakeFiles/CreateJson.dir/src/make_test_json_file.cc.o
.PHONY : src/make_test_json_file.cc.o

src/make_test_json_file.i: src/make_test_json_file.cc.i

.PHONY : src/make_test_json_file.i

# target to preprocess a source file
src/make_test_json_file.cc.i:
	$(MAKE) -f CMakeFiles/CreateJson.dir/build.make CMakeFiles/CreateJson.dir/src/make_test_json_file.cc.i
.PHONY : src/make_test_json_file.cc.i

src/make_test_json_file.s: src/make_test_json_file.cc.s

.PHONY : src/make_test_json_file.s

# target to generate assembly for a file
src/make_test_json_file.cc.s:
	$(MAKE) -f CMakeFiles/CreateJson.dir/build.make CMakeFiles/CreateJson.dir/src/make_test_json_file.cc.s
.PHONY : src/make_test_json_file.cc.s

src/server.o: src/server.cc.o

.PHONY : src/server.o

# target to build an object file
src/server.cc.o:
	$(MAKE) -f CMakeFiles/ser.dir/build.make CMakeFiles/ser.dir/src/server.cc.o
.PHONY : src/server.cc.o

src/server.i: src/server.cc.i

.PHONY : src/server.i

# target to preprocess a source file
src/server.cc.i:
	$(MAKE) -f CMakeFiles/ser.dir/build.make CMakeFiles/ser.dir/src/server.cc.i
.PHONY : src/server.cc.i

src/server.s: src/server.cc.s

.PHONY : src/server.s

# target to generate assembly for a file
src/server.cc.s:
	$(MAKE) -f CMakeFiles/ser.dir/build.make CMakeFiles/ser.dir/src/server.cc.s
.PHONY : src/server.cc.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... CreateJson"
	@echo "... rebuild_cache"
	@echo "... redo"
	@echo "... ser"
	@echo "... cli"
	@echo "... CJsonTool"
	@echo "... demo2"
	@echo "... demo"
	@echo "... demo1"
	@echo "... src/Server.o"
	@echo "... src/Server.i"
	@echo "... src/Server.s"
	@echo "... src/client.o"
	@echo "... src/client.i"
	@echo "... src/client.s"
	@echo "... src/make_test_json_file.o"
	@echo "... src/make_test_json_file.i"
	@echo "... src/make_test_json_file.s"
	@echo "... src/server.o"
	@echo "... src/server.i"
	@echo "... src/server.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

