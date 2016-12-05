#!/bin/bash

errexit()
{
    echo Error: $1
    exit 1
}


function build()
{
  pushd dataformats/cncs2016
  make || errexit "failed to build"
  popd
}
