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
}

function artifacts()
{
  pushd artifacts
  tar xzvf librdkafka-0.9.1.tar.gz
  popd
}

function libs() 
{
  pushd libs
  make V=y 
  popd
}

function libs_test() 
{
  pushd libs
  make V=y GTEST=../artifacts/code/googletest/build/usr/local test
  make V=y runtest
  popd
}

function prototype()
{
  pushd prototype2
  make V=y NOKAFKA=y
  popd
}

function prototype_test()
{
  pushd prototype2
  make V=y GTEST=../artifacts/code/googletest/build/usr/local test
  make V=y runtest
  make doxygen
  popd
}

tools
artifacts
libs
prototype
libs_test
prototype_test
