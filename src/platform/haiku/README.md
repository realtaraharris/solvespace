# building
cd build
cmake .. -D ENABLE_GUI=YES -D ENABLE_TESTS=NO -D FORCE_VENDORED_Eigen3=YES
cmake --build .

# testing
cd build
make test-solvespace

# clean build
cmake --build . --clean-first
