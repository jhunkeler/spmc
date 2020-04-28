#!/bin/bash
if [[ $(uname -s) == Linux ]]; then
    yum install -y \
        make \
        patchelf \
        binutils \
        curl-devel \
        openssl-devel \
        file \
        which \
        rsync \
        tar \
        cmake3 \
        gcc \
        gcc-c++ \
        gcc-gfortran \
        glibc-devel \
        libxslt

    ln -s cmake3 /usr/bin/cmake
    ln -s ctest3 /usr/bin/ctest

elif [[ $(uname -s) == Darwin ]]; then
    brew install cmake \
        gnu-tar \
        openssl@1.1
fi
