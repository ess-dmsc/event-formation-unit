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
        echo $file
        head -n 1 $file | grep "Copyright" &>/dev/null || err "No copyright" 
    done
    echo
}

function doxygen()
{
    echo Checking for Doxygen comments

    FILES=$(find . -name "*.h")
    for file in $FILES; 
    do
        echo $file
        grep "@file" $file &>/dev/null || err "Missing @file description" 
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
