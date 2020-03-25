#!/bin/bash
yum install -y epel-release
yum install -y git \
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
    glibc-devel
