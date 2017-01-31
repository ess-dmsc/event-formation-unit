#!/bin/bash

function err()
{
    ERRORS=$((ERRORS + 1))
    echo Error: $1
}

function copyright()
{
    echo Checking for Copyright notice

    FILES=$(find . -name "*.cpp" -o -name "*.c" -o -name "*.h")
    for file in $FILES; 
    do
        head -n 1 $file | grep "Copyright" &>/dev/null || err "No copyright in $file"
        head -n 1 $file | grep "Copyright" | grep "2017" &>/dev/null || err "Copyright missing 2017 in $file"
    done
    echo
}

function doxygen()
{
    echo Checking for Doxygen comments

    FILES=$(find . -name "*.h")
    for file in $FILES; 
    do
        grep "@file" $file &>/dev/null || err "Missing @file description in $file"
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
