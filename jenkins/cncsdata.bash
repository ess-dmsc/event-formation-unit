#!/bin/bash

errexit()
{
    echo Error: $1
    exit 1
}


rm -fr build
mkdir build

pushd build

/opt/dm_group/virtualenv/conan/bin/conan install --build=outdated ..
source activate_run.sh
cmake ../dataformats  || errexit "cmake failed"
make runtest          || errexit "make runtest failed"

popd
