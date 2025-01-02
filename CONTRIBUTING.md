# Building

```
cd build
cmake ..
cmake --build . -j --target solvespace
```

# Testing

```
cd build
make test-solvespace
```

# clean build

```
cd build
cmake --build . --clean-first
```

# cmake targets:

- clang-format
- clang-tidy
- solvespace-debugtool
- solvespace-testsuite
- solvespace-core
- solvespace
- test-solvespace

Contributing to SolveSpace for Haiku
====================================

Contributing bug reports
------------------------

Bug reports are always welcome! When reporting a bug, please include the following:

  * The version of SolveSpace (use Help → About...); [[TODO: Add help menu]]
  * The Haiku version (hrev# if running a nightly);
  * The save file that reproduces the incorrect behavior, or, if trivial or impossible,
    instructions for reproducing it.

GitHub does not allow attaching `*.slvs` files, but it does allow attaching `*.zip` files,
so any savefiles should first be archived.

Licensing
---------

SolveSpace for Haiku includes portions of SolveSpace, which is licensed under the GPLv3. All new
contributions must comply with this license, but there's no reason why your contribution can't be
dual-licensed under both GPLv3 and another (hopefully permissive) one.

Contributing translations
-------------------------

Feel free to contribute translations, making sure they are capitalized and otherwise written to
follow Haiku-specific user interface guidelines and conventions. Translation support isn't ready in
the code, but it will rely on Haiku-specific translation APIs when it is. Note that there are places
where strings will differ from the ones found upstream in SolveSpace due to improvements made in
this version.

Contributing code
-----------------

SolveSpace for Haiku is written in C++, and currently targets recent versions of GCC and Clang.

### High-level conventions

SolveSpace for Haiku aims to refactor the original SolveSpace core to cleanly separate it into even
smaller parts. The first step (now halfway complete) is to completely remove the existing cross-
platform UI code and replace it with native Haiku API interface code in `src/platform/ui/haiku/`.

The next step will involve a refactor of the core to improve its navigability.

New rules:
* one class definition per file, except for small groups (e.g., subclasses of VectorFileWriter)
* the file name must match the class name
* reduce the use of macros

If the original SolveSpace project makes significant progress on improving the core first, we may
want to absorb those changes first before refactoring.

#### Portability

Shorter CMake files are more important than supporting C++ compilers outside of GCC and Clang.

A major purpose of this project is to drive more users to use the Haiku operating system. Obviously
you can use this code as a stepping stone for well-integrated ports to Qt, Windows or macOS, but
please consider spending that time on improving Haiku instead. The world needs a friendly
open-source OS with a consistent user interface that creators can use comfortably and indefinitely,
free of corporate control.