#!/bin/bash
mkdir build
cd build
cmake3 -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Debug ..
make install
