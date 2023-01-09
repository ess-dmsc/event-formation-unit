#!/bin/bash

DIR=$1
#mydir=$(pwd)

if [[ $DIR == "" ]] ; then
  exit 0
fi

files=$(find $DIR -type f -regex '.*Test\.[ch]p*')
#files=$(find $DIR -type d -name test| sed 's/^\.\.\///')

EXCL=""
for file in $files
do
    EXCL=$EXCL" -i$file"
done

echo $EXCL
