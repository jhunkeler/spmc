#!/bin/bash
cd build

set -x
ctest3 -V

spm --list

spm --search zlib
