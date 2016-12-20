#!/bin/bash

errexit()
{
    echo Error: $1
    exit 1
}


function tools()
{
  g++ --version
  gcov --version
  lcov --version
  valgrind --version
  doxygen --version
  dot -V
  lscpu
  gcovr --version
}

function libs()
{
  pushd libs
  make V=y || errexit "libs build failed"
  popd
}

function libs_test()
{
  pushd libs
  #make V=y $1 GTEST=../artifacts/code/googletest/build/usr/local test || errexit "unable to build libs tests"
  make V=y $1 test || errexit "unable to build libs tests"
  make V=y runtest
  popd
}

function prototype()
{
  pushd prototype2
  make V=y KAFKA_ROOT=../artifacts/librdkafka || errexit "prototype2 build failed"
  popd
}

function prototype_test()
{
  pushd prototype2
  #make V=y $1 GTEST=../artifacts/code/googletest/build/usr/local test || errexit "unable to build prototype2 tests"
  make V=y $1 test || errexit "unable to build prototype2 tests"
  make V=y runtest
  make doxygen
  popd
}

function coverage()
{
  mkdir -p gcovr
  gcovr -r . -x -e '.*Test.cpp' -e '.*gtest.*.h' -o gcovr/coverage.xml
  gcovr -r . --html --html-details -e '.*Test.cpp' -e '.*gtest.*.h' -o coverage.html
}


tools
libs
prototype
libs_test COV=y
prototype_test COV=y
coverage
