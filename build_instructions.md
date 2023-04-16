git submodule update --init extlib/libdxfrw extlib/mimalloc
pkgman install freetype_devel cairo_devel eigen json_c_devel agg_devel freetype_debuginfo fontconfig_debuginfo graphite2_debuginfo libiconv_debuginfo lcms_debuginfo
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_GUI=off ..
cmake --build .
./bin/SolveSpace

try opening a test file using the file menu. i.e., solvespace/test/analysis/contour_area/normal.slvs