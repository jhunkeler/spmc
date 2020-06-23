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
- bsdtar (https://www.libarchive.org)
  - OR gnutar (https://www.gnu.org/software/tar)
- which (https://carlowood.github.io/which)

## Runtime Requirements

- file (http://darwinsys.com/file)
- patchelf (https://nixos.org/patchelf.html)
- objdump (https://www.gnu.org/software/binutils)
- reloc (https://github.com/jhunkeler/reloc)
- rsync (https://rsync.samba.org)
- bsdtar (https://www.libarchive.org)
  - OR gnutar (https://www.gnu.org/software/tar)
- which (https://carlowood.github.io/which)

## Installation

### Dependencies

#### CentOS 7+

```bash
$ yum install epel-release
$ yum install -y binutils cmake3 curl-devel file gcc gcc-c++ gcc-gfortran glibc-devel \
    make openssl-devel patchelf rsync bsdtar which
```

#### Arch

```bash
$ pacman -S binutils cmake curl file gcc gcc-c++ gcc-gfortran openssl make patchelf \
    rsync libarchive which
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

### Developing SPM

```bash
$ git clone https://github.com/jhunkeler/spm
$ cd spm
$ mkdir build
$ cmake ..
```

## Configuration

_TODO_

## Usage

```bash
$ spm --help
usage: spm [-hVvBIRrmMLS]
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
       
$ spm --cmd
possible commands:
  mkprefixbin          - generate prefix manifest (binary)
  mkprefixtext         - generate prefix manifest (text)
  mkmanifest           - generate package repository manifest
  mkruntime            - emit runtime environment (stdout)
  mirror_clone         - mirror a mirror     
  rpath_set            - modify binary RPATH 
  rpath_autoset        - determine nearest lib directory and set RPATH
  get_package_ext      - show the default archive extension
  get_sys_target       - show this system's arch/platform
  check_rt_env         - check the integrity of the calling runtime environment       
```

### Example

#### List available packages
```bash
$ spm --list
#-------------------------------------------------------------------------------
# name               version        size     origin
#-------------------------------------------------------------------------------
autoconf             2.69-0      883.65K     https://astroconda.org/spm/Linux/x86_64
automake             1.16.1-0    656.70K     https://astroconda.org/spm/Linux/x86_64
binutils             2.34-0        6.33M     https://astroconda.org/spm/Linux/x86_64
bison                3.4.2-0     751.46K     https://astroconda.org/spm/Linux/x86_64
bzip2                1.0.8-0     229.04K     https://astroconda.org/spm/Linux/x86_64
cmake                3.15.5-0     14.37M     https://astroconda.org/spm/Linux/x86_64
curl                 7.66.0-0      1.01M     https://astroconda.org/spm/Linux/x86_64
diffutils            3.7-0       547.74K     https://astroconda.org/spm/Linux/x86_64
e2fsprogs            1.45.4-0     73.35K     https://astroconda.org/spm/Linux/x86_64
filesystem           1.0.0-0        567B     https://astroconda.org/spm/Linux/x86_64
findutils            4.7.0-0     777.81K     https://astroconda.org/spm/Linux/x86_64
gcc                  8.4.0-0      57.89M     https://astroconda.org/spm/Linux/x86_64
# [...]
```

#### Search for a package
```bash
$ spm --search python
#-------------------------------------------------------------------------------
# name               version        size     origin
#-------------------------------------------------------------------------------
python               3.8.2-0      26.05M     https://astroconda.org/spm/Linux/x86_64
```

#### Install a package
```bash
$ spm --root ~/spmenv123 --install "python" # [...]
```

#### Export environment variables

```bash
$ spm --mkruntime ~/spmenv123 > ~/spmenv123/bin/activate
$ source ~/spmenv123/bin/activate
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
