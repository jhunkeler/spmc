# Simple Package Manager (SPM)

(Original project: https://github.com/jhunkeler/spm)

A basic userland package management system with a few humble goals:

- To be simple and easy to use
- To support Linux, MacOS (not yet), and Windows (not yet)
- To avoid using `root` (or administrative accounts in general) to install software
- To keep your Monday mornings and Fridays evenings pain free

## Build requirements

- cmake (https://cmake.org)
- curl (https://curl.haxx.se)
- gcc (https://gcc.gnu.org)
- openssl (https://www.openssl.org)

## Runtime Requirements

- file (http://darwinsys.com/file)
- patchelf (https://nixos.org/patchelf.html)
- reloc (https://github.com/jhunkeler/reloc)
- rsync (https://rsync.samba.org)
- tar (https://www.gnu.org/software/tar)

## Installation

### Dependencies

#### CentOS

```bash
$ yum install epel-release
$ yum install cmake curl-devel file gcc openssl-devel patchelf rsync tar
```

#### Arch

```bash
$ pacman -S cmake curl file gcc openssl patchelf rsync tar
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
usage: spm [-hVvBIrLS]
  -h,  --help     show this help message
  -V,  --version  show version
  -v,  --verbose  show more information
  -B,  --build    build package(s)
  -I,  --install  install package(s)
  -r,  --root     installation prefix (requires --install)
  -L,  --list     list available packages
  -S,  --search   search for a package
```

### Example

#### Install Python
```bash
$ spm --root ~/spmenv123 --install "python" # [...]
```

#### Export environment variables

```bash
$ export PATH="~/spmenv123/bin:$PATH"
$ export MANPATH="~/spmenv123/share/man:$MANPATH"
$ hash -r  # or "rehash" if your shell supports it
```

#### Use Python

```bash
$ which python
/home/example/spmenv123/bin/python
$ which pip
/home/example/spmenv123/bin/pip
$ which git
/home/example/spmenv123/bin/git

$ python -m venv ~/spmenv123/venv
$ source ~/spmenv123/venv/bin/activate
$ pip install https://github.com/spacetelescope/jwst.git#egg=jwst
# ... do work
```

## Building SPM Packages

_TODO_

## Development

```bash
$ git clone https://github.com/jhunkeler/spm
$ cd spm
$ mkdir build
$ make
```


