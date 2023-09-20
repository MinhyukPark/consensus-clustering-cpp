#!/bin/sh
rm -r build
mkdir build
cd build
cmake ..
cp compile_commands.json ../
make -j 1
