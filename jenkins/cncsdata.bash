#!/bin/bash

errexit()
{
    echo Error: $1
    exit 1
}


rm -fr build
mkdir build

pushd build

cmake ../dataformats  || errexit "cmake failed"
make runtest          || errexit "make runtest failed"

popd
