#!/bin/sh
set -eu
# Run from repository root; requires C/C++ compilers, make, CMake, tar.
mkdir -p build-deps
if [ ! -f build-deps/install/lib/libsodium.a ]; then
  tar -xzf vendor/libsodium-1.0.20.tar.gz -C build-deps
  task_root=$(pwd)
  (cd build-deps/libsodium-1.0.20 && ./configure --disable-shared --prefix="$task_root/build-deps/install" && make -j2 && make install)
fi
cmake -S . -B build -DSODIUM_ROOT="$(pwd)/build-deps/install" -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
