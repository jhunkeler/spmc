# Simple Package Manager (SPM)

[![CircleCI](https://circleci.com/gh/jhunkeler/spmc/tree/master.svg?style=svg)](https://circleci.com/gh/jhunkeler/spmc/tree/master)

A basic userland package management system with a few humble goals:

- To be simple and easy to use
- To support Linux, MacOS (not yet), and Windows (not yet)
- To avoid using `root` (or administrative accounts in general) to install software
- To keep your Monday mornings and Friday evenings pain free

## Build requirements

- cmake (https://cmake.org)
- curl (https://curl.haxx.se)
- gcc (https://gcc.gnu.org)
- make (https://www.gnu.org/software/make)
- openssl (https://www.openssl.org)
- tar (https://www.gnu.org/software/tar)
- which (https://carlowood.github.io/which)

## Runtime Requirements

- file (http://darwinsys.com/file)
- patchelf (https://nixos.org/patchelf.html)
- objdump (https://www.gnu.org/software/binutils)
- reloc (https://github.com/jhunkeler/reloc)
- rsync (https://rsync.samba.org)
- tar (https://www.gnu.org/software/tar)
- which (https://carlowood.github.io/which)

## Installation

### Dependencies

#### CentOS 7+

```bash
$ yum install epel-release
$ yum install -y binutils cmake3 curl-devel file gcc gcc-c++ gcc-gfortran glibc-devel \
    make openssl-devel patchelf rsync tar which
```

#### Arch

```bash
$ pacman -S binutils cmake curl file gcc gcc-c++ gcc-gfortran openssl make patchelf \
    rsync tar which
```

### Install reloc

```bash
$ git clone https://github.com/jhunkeler/reloc
$ cd reloc
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release
```

### Install SPM

```bash
$ git clone https://github.com/jhunkeler/spmc
$ cd spmc
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release
$ make install
```

## Configuration

_TODO_

## Usage

```bash
$ spm --help
usage: spm [-hVvBIrmMLS]
  -h,  --help                show this help message
  -V,  --version             show version
  -v,  --verbose             show more information (additive)
  -y   --yes                 do not prompt
  -B,  --build               build package(s)
  -I,  --install             install package(s)
  -R   --remove              remove package(s)
  -r,  --root                installation prefix (requires --install)
  -m   --manifest            specify a package manifest to use
  -M   --override-manifests  disable default package manifest location
  -L,  --list                list available packages
  -S,  --search              search for a package
       --cmd                 execute an internal spm command
```

### Example

#### List available packages
```bash
$ spm --list
#-------------------------------------------------------------------------------
# name                 version    revision   size       origin
#-------------------------------------------------------------------------------
  autoconf             2.69       0          885.33K    https://astroconda.org/spm
  automake             1.16.1     0          742.90K    https://astroconda.org/spm
  binutils             2.34       0          7.64M      https://astroconda.org/spm
  bison                3.4.2      0          754.43K    https://astroconda.org/spm
  bzip2                1.0.8      0          130.84K    https://astroconda.org/spm
  cfitsio              3.47       0          1.17M      https://astroconda.org/spm
  curl                 7.66.0     0          1.02M      https://astroconda.org/spm
  e2fsprogs            1.45.4     0          72.66K     https://astroconda.org/spm
  filesystem           1.0.0      0          578B       https://astroconda.org/spm
  findutils            4.7.0      0          772.41K    https://astroconda.org/spm
  gcc                  8.4.0      0          60.08M     https://astroconda.org/spm
# [...]
```

#### Search for a package
```bash
$ spm --search python
#-------------------------------------------------------------------------------
# name                 version    revision   size       origin
#-------------------------------------------------------------------------------
  python               3.8.2      0          26.05M     https://astroconda.org/spm
```

#### Install a package
```bash
$ spm --root ~/spmenv123 --install "python" # [...]
```

#### Export environment variables

```bash
$ source <(spm --mkruntime ~/spmenv123)
$ hash -r  # or "rehash" if your shell supports it
```

#### Use package

```bash
$ python -m ensurepip
$ python -m venv ~/spmenv123/venv
$ source ~/spmenv123/venv/bin/activate
(venv) $ pip install https://github.com/spacetelescope/jwst.git#egg=jwst
# ... do work
```

## Building SPM Packages

See the [spm_packages](https://github.com/jhunkeler/spm_packages) repository.

## Development

```bash
$ git clone https://github.com/jhunkeler/spm
$ cd spm
$ mkdir build
$ cmake ..
```


