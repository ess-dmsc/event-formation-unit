#!/bin/bash

dir=$1

if [[ $1 == "" ]] ; then
    echo usage: makelist directory
    exit 0
fi

find $1  -type f -regex ".*\.[ch]p*"  | grep -v "Test\.[ch]p*$"
