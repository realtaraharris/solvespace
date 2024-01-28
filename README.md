# SolveSpace for Haiku

<img src="src/platform/haiku/resources/icons/solvespace.png" width="70" height="70" alt="SolveSpace for Haiku Logo" align="left">

This repository contains the source code for SolveSpace for Haiku, a parametric 2D/3D CAD with a native Haiku user interface.

Github: [Solvespace for Haiku](https://github.com/realtaraharris/solvespace)

## Installation

### Via Official Packages

Official packages don't exist yet, but will soon.

### Via source code

```sh
git submodule update --init extlib/libdxfrw
pkgman install freetype_devel cairo_devel eigen json_c_devel agg_devel freetype_debuginfo fontconfig_debuginfo graphite2_debuginfo libiconv_debuginfo lcms_debuginfo
mkdir build
cd build
cmake .. -D ENABLE_GUI=YES -D ENABLE_TESTS=NO -D FORCE_VENDORED_Eigen3=YES
cmake --build .
./bin/SolveSpace
```

Try opening a test file using the file menu. i.e., solvespace/test/analysis/contour_area/normal.slvs

## Contributing

See the [guide for contributors](CONTRIBUTING.md) for the best way to file issues, contribute code,
and debug SolveSpace for Haiku.

## License

SolveSpace for Haiku is distributed under the terms of the [GPL v3](COPYING.txt) or later. The new Haiku-specific user interface code is dual-licensed GPL v3 and MIT.