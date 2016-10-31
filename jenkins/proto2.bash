#!/bin/bash


function tools()
{
  g++ --version
  gcov --version 
  lcov --version
  valgrind --version
  dot --version
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
  make V=y GTEST=../artifacts/code/googletest/build/usr/local test
  make V=y runtest
  popd
}

function prototype()
{
  pushd prototype2
  make V=y NOKAFKA=y
  make V=y GTEST=../artifacts/code/googletest/build/usr/local test
  make V=y runtest
  #make doxygen
  popd
}

#tools
artifacts
#libs
prototype
