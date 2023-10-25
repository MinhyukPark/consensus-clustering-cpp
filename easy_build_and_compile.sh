#!/bin/sh
rm ./consensus_clustering
rm -r build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cp compile_commands.json ../
make -j 1
ln -s $(readlink -f ./bin/consensus_clustering) ../consensus_clustering
