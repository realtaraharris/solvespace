# cmake configuration
cmake_minimum_required(VERSION 3.30)

# TODO: remove hardcoded path that only works on my machine
add_definitions(-g -gdwarf-4 -Wno-deprecated-enum-float-conversion -Wno-conversion-null -Wno-deprecated-declarations -Wno-deprecated-enum-enum-conversion -Wno-register -I/boot/home/cppfront/include/ -fno-diagnostics-show-option)

list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_SOURCE_DIR}/cmake/")

# this project won't compile when it emits -std=g++20, so let's try to force it to always use clang
#SET(CMAKE_C_COMPILER "/bin/clang")
#SET(CMAKE_CXX_COMPILER "/bin/clang++")

if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

# project

# NOTE TO PACKAGERS: The embedded git commit hash is critical for rapid bug triage when the builds
# can come from a variety of sources. If you are mirroring the sources or otherwise build when
# the .git directory is not present, please comment the following line:
include(GetGitCommitHash)
# and instead uncomment the following, adding the complete git hash of the checkout you are using:
# set(GIT_COMMIT_HASH 0000000000000000000000000000000000000000)

string(SUBSTRING "${GIT_COMMIT_HASH}" 0 8 solvespace_GIT_HASH)
project(solvespace
    VERSION 3.1
    LANGUAGES C CXX ASM)

set(ENABLE_GUI        ON CACHE BOOL
    "Whether the graphical interface is enabled")
set(ENABLE_CLI        ON CACHE BOOL
    "Whether the command line interface is enabled")
set(ENABLE_TESTS      ON  CACHE BOOL
    "Whether the test suite will be built and run")
set(ENABLE_COVERAGE   OFF CACHE BOOL
    "Whether code coverage information will be collected")
set(ENABLE_SANITIZERS OFF CACHE BOOL
    "Whether to enable Clang's AddressSanitizer and UndefinedBehaviorSanitizer")
set(ENABLE_OPENMP     OFF CACHE BOOL
    "Whether geometric operations will be parallelized using OpenMP")
set(ENABLE_LTO        OFF CACHE BOOL
    "Whether interprocedural (global) optimizations are enabled")
option(FORCE_VENDORED_Eigen3
    "Whether we should use our bundled Eigen even in the presence of a system copy"
    OFF)

set(OPENGL 3 CACHE STRING "OpenGL version to use (one of: 1 3)")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH    ${CMAKE_BINARY_DIR}/bin)
if("${CMAKE_GENERATOR}" STREQUAL "Xcode")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
        # GCC 4.8/4.9 ship with broken but present <regex>. meh.
        message(FATAL_ERROR "GCC 5.0+ is required")
    endif()
endif()

#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_COLOR_DIAGNOSTICS OFF)
set(FETCHCONTENT_QUIET FALSE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# common compiler flags
#include(CheckCXXCompilerFlag)

#set(CMAKE_CXX_FLAGS -std=c++20)
#add_compile_options("-std=c++20")

# Ensure that all platforms use 64-bit IEEE floating point operations for consistency;
# this is most important for the testsuite, which compares savefiles directly
# and depends on consistent rounding of intermediate results.
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "i686" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "X86")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(FLOAT_FLAGS "-mfpmath=sse -msse2")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(FLOAT_FLAGS "-msse2")
    endif()

    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${FLOAT_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLOAT_FLAGS}")
endif()

if(ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported()
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

if(ENABLE_OPENMP)
    find_package( OpenMP REQUIRED )
    if(OPENMP_FOUND)
        add_library(slvs_openmp INTERFACE)
        target_compile_options(slvs_openmp INTERFACE ${OpenMP_CXX_FLAGS})
        target_link_libraries(slvs_openmp INTERFACE
            ${OpenMP_CXX_LIBRARIES})
        target_include_directories(slvs_openmp INTERFACE SYSTEM
            ${OpenMP_CXX_INCLUDE_DIRS})
        message(STATUS "found OpenMP, compiling with flags: " ${OpenMP_CXX_FLAGS} )
    endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    set(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed ${CMAKE_EXE_LINKER_FLAGS}")
endif()

if(ENABLE_SANITIZERS)
    if(NOT SANITIZERS)
        set(SANITIZERS "address;undefined")
    endif()

    if("thread" IN_LIST SANITIZERS)
        list(REMOVE_ITEM SANITIZERS "thread")
        list(APPEND SANITIZE_OPTIONS thread)
    endif()
    if("address" IN_LIST SANITIZERS)
        list(REMOVE_ITEM SANITIZERS "address")
        list(APPEND SANITIZE_OPTIONS address)
    endif()
    if("undefined" IN_LIST SANITIZERS)
        list(REMOVE_ITEM SANITIZERS "undefined")
        list(APPEND SANITIZE_OPTIONS alignment bounds)
        list(APPEND SANITIZE_OPTIONS shift signed-integer-overflow integer-divide-by-zero)
        list(APPEND SANITIZE_OPTIONS null bool enum)
        list(APPEND SANITIZE_OPTIONS return)
    endif()
    if(SANITIZERS)
        message(FATAL_ERROR "Unknown sanitizer(s) ${SANITIZERS}")
    else()
        message(STATUS "Using sanitizer options ${SANITIZE_OPTIONS}")
    endif()

    string(REPLACE ";" "," SANITIZE_OPTIONS "${SANITIZE_OPTIONS}")

    if (NOT APPLE)
        set(SANITIZE_FLAGS "-O1 -fsanitize=${SANITIZE_OPTIONS} -fno-sanitize-recover=address,undefined")
    else()
        set(SANITIZE_FLAGS "-O1 -fsanitize=${SANITIZE_OPTIONS}")
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(SANITIZE_FLAGS "${SANITIZE_FLAGS} -fno-omit-frame-pointer -fno-optimize-sibling-calls")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(SANITIZE_FLAGS "${SANITIZE_FLAGS} -fuse-ld=gold")
    else()
        message(FATAL_ERROR "Sanitizers are only available when using GCC or Clang")
    endif()

    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${SANITIZE_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZE_FLAGS}")
endif()

# common dependencies

message(STATUS "Using in-tree libdxfrw")
add_subdirectory(extlib/libdxfrw)

set(MI_OVERRIDE OFF CACHE BOOL "")
set(MI_BUILD_SHARED OFF CACHE BOOL "")
set(MI_BUILD_OBJECT OFF CACHE BOOL "")
set(MI_BUILD_TESTS OFF CACHE BOOL "")

if(NOT FORCE_VENDORED_Eigen3)
    find_package(Eigen3 CONFIG)
endif()
if(FORCE_VENDORED_Eigen3 OR NOT EIGEN3_INCLUDE_DIRS)
    message(STATUS "Using in-tree Eigen")
    set(EIGEN3_FOUND             YES)
    set(EIGEN3_INCLUDE_DIRS      ${CMAKE_SOURCE_DIR}/extlib/eigen)
else()
    message(STATUS "Using system Eigen: ${EIGEN3_INCLUDE_DIRS}")
endif()
if(NOT EXISTS "${EIGEN3_INCLUDE_DIRS}")
    message(FATAL_ERROR "Eigen 3 not found on system or in-tree")
endif()


# On Linux and BSDs we're a good citizen and link to system libraries.
find_package(PkgConfig REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(Freetype REQUIRED)
pkg_check_modules(CAIRO REQUIRED cairo)
list(APPEND AGG_INCLUDE_DIRS /boot/system/develop/headers/agg2/)

# GUI dependencies

if(ENABLE_GUI)
#    find_package(OpenGL REQUIRED)
    add_compile_definitions(HAIKU_GUI=${HAIKU_GUI})
endif()

# code coverage

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
        find_program(GCOV gcov)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES Clang)
        find_program(LLVM_COV llvm-cov)

        if(LLVM_COV)
            set(GCOV ${CMAKE_CURRENT_BINARY_DIR}/llvm-gcov.sh)
            file(WRITE ${GCOV} "#!/bin/sh -e\n${LLVM_COV} gcov $*")
            execute_process(COMMAND chmod +x ${GCOV})
        endif()
    endif()

    find_program(LCOV lcov)
    find_program(GENHTML genhtml)
    if(NOT GCOV OR NOT LCOV OR NOT GENHTML)
        message(FATAL_ERROR "gcov/llvm-cov and lcov are required for producing coverage reports")
    endif()

    if(NOT (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
            CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
        message(FATAL_ERROR "Code coverage is only available on GCC and Clang")
    endif()

    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(FATAL_ERROR "Code coverage produces reliable results only on Debug builds")
    endif()

    # With -fexceptions, every call becomes a branch. While technically accurate,
    # this is not useful for us.
    set(COVERAGE_FLAGS -fno-exceptions --coverage)
    set(COVERAGE_LIBRARY --coverage)
endif()

# application components

add_subdirectory("src/platform/haiku")
add_subdirectory(res)
add_subdirectory(src)
#add_subdirectory(exposed)

if(ENABLE_TESTS)
    add_subdirectory(test)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_subdirectory(bench)
else()
    message(STATUS "Benchmarking disabled in debug builds.")
endif()
