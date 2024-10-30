#!/bin/bash
/bin/c++ -I/boot/system/develop/headers/agg2 -I/boot/home/solvespace -I/boot/home/solvespace/src/platform/haiku/../../../src -I/boot/home/solvespace/src/platform/haiku/../../src -I/boot/home/solvespace/src -I/boot/home/solvespace/src/platform/haiku/ui -I/boot/home/cppfront/include -std=c++20 -Wno-register -Wno-deprecated-declarations -Wno-deprecated-enum-float-conversion -Wno-deprecated-enum-enum-conversion ../src/vector-test.cpp \
../src/vector.cpp \
-o /boot/home/solvespace/build/tcheck
