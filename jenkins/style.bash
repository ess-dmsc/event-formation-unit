#!/bin/bash

#cmd=${1:-all}


function err()
{
    ERRORS=$((ERRORS + 1))
    echo Error: $1
}

function copyright()
{
  echo Checking for Copyright notice

  files=$(find . -name "*.cpp" -o -name "*.c" -o -name "*.h" -o -name "*.hpp")
    for file in $files;
    do
        head -n 1 $file | grep "Copyright" &>/dev/null || err "No copyright in $file"
    done
    echo
}

function doxygen()
{
    echo Checking for Doxygen comments

  files=$(find . -name "*.h")
    for file in $files;
    do
        grep '/// \\file' $file &>/dev/null || err "Missing /// \file description in $file"
    done
    echo
}

ERRORS=0
copyright
doxygen

if [[ $ERRORS != "0" ]]; then
    echo Error: checks failed, exiting...
    exit 1
fi
