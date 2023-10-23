#!/bin/bash

mkdir -p build
cd build
cmake ..
cmake --build .
cd ..
mv build/daemon daemon
rm -rf build
