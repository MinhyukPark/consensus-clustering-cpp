#!/bin/sh

external_libs_full_path=$(readlink -f ./external_libs)
rm -rf ${external_libs_full_path}
mkdir ${external_libs_full_path}
cd ${external_libs_full_path}
# install igraph to external_libs
echo "installing igraph to ${external_libs_full_path}"
git clone https://github.com/igraph/igraph.git
cd igraph
mkdir build
cd build
# to avoid linking shared
export CXXFLAGS="$CXXFLAGS -fPIC"
cmake .. -DCMAKE_INSTALL_PREFIX=${external_libs_full_path}
cmake --build .
cmake --install .

# install libleidenalg to external_libs
cd ${external_libs_full_path}
echo "installing libleidenalg to ${external_libs_full_path}"
git clone https://github.com/vtraag/libleidenalg.git
cd libleidenalg
mkdir build
cd build
# to avoid linking shared
export CXXFLAGS="$CXXFLAGS -fPIC"
cmake .. -DCMAKE_INSTALL_PREFIX=${external_libs_full_path} -DCMAKE_PREFIX_PATH=${external_libs_full_path}
cmake --build .
cmake --build . --target install
