# building
cd build
cmake .. -D ENABLE_GUI=NO -D ENABLE_TESTS=YES -D FORCE_VENDORED_Eigen3=YES
cmake --build .

# testing
cd build
make test_solvespace

# clean build
cmake --build . --clean-first
