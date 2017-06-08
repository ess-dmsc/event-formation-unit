#!/bin/bash

errexit()
{
    echo Error: $1
    exit 1
}


function tools()
{
  g++ --version || errexit "g++ program is missing"
  gcov --version || errexit "gcov program is missing"
  valgrind --version  || errexit "valgrind program is missing"
  doxygen --version  || errexit "doxygen program is missing"
  dot -V  "dot program is missing"
  lscpu
  gcovr --version || errexit "gcovr program is missing"
}

rm -fr build
mkdir build

#tools
cloc --by-file --xml --out=cloc.xml .
pushd build
cmake ..
make
popd
