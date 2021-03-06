#!/bin/bash -e
source $(dirname "${BASH_SOURCE[0]}")/runtime.sh

export TEST_RESULTS=${TEST_RESULTS:-/tmp/test-results}
export PREFIX=/tmp/root
export SHELL=/bin/bash
mkdir -p "${TEST_RESULTS}"/spm
cd build

set -x

ctest -T test
xsltproc \
    ../.circleci/ctest-to-junit.xsl \
    Testing/$(head -n 1 < Testing/TAG)/Test.xml > "${TEST_RESULTS}"/spm/Test.xml

spm --list

spm --search zlib

spm --verbose --yes --root $PREFIX --install python

set +x
echo ACTIVATING ROOT: $PREFIX
spm --cmd mkruntime $PREFIX > $PREFIX/bin/activate || exit 1
source $PREFIX/bin/activate || exit 1
echo OK!
set -x

which python3

python3 -V

python3 -c 'from sysconfig import get_config_vars; from pprint import pprint; pprint(get_config_vars())'

python3 -m ensurepip

pip3 --version

pip3 install --upgrade pip setuptools

pip3 --version
