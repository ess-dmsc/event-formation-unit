#!/bin/bash

errexit()
{
    echo Error: $1
    exit 1
}


function build_progs()
{
  pushd dataformats/multigrid
  make V=y || errexit "failed to build progs"
  popd
}

function build_tests()
{
  pushd dataformats/multigrid
  make V=y test || errexit "failed to build tests"
  popd
}

function run_tests()
{
  pushd dataformats/multigrid
  make V=y runtest || errexit "failed to run tests"
  popd
}

build_progs
build_tests
run_tests
