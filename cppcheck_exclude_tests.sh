#!/bin/bash

DIR=$1

# find all test files (assuming they end in Test.cpp/h/hpp
# then make command line arguments for cppcheck of the form
# -ipath/to/testfile.cpp -ipath/to/other/testfile.cpp ...

# Used in the cppcheck call for cppcheck target and for Jenkins builds

# empty list is fine
if [[ $DIR == "" ]] ; then
  exit 0
fi

files=$(find $DIR -type f -regex '.*Test\.[ch]p*')

excl=""
for file in $files
do
    excl=$excl" -i$file"
done

echo $excl
