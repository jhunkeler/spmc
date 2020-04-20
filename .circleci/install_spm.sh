#!/bin/bash
source $(dirname "${BASH_SOURCE[0]}")/runtime.sh

mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Debug ..
make install
