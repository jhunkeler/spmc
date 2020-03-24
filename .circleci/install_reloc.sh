#!/bin/bash
git clone https://github.com/jhunkeler/reloc
mkdir -p reloc/build
pushd reloc/build
    cmake3 -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release ..
    make install
popd
rm -rf reloc
