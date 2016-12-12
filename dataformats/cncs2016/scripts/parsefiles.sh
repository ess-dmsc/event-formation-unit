#!/bin/bash

DIR="/home/morten/nfs/data/raw/MG_CNCS/07_25/"

#./batchreader -d $DIR -p 2016_07_25_1051_sample_ -o .bin -s 0 -e  1365 -f f1051 
./batchreader -d $DIR -p 2016_07_26_1005_sample_ -o .bin -s 0 -e  1442 -f f1005 
#./batchreader -d $DIR -p 2016_07_27_1113_sample_ -o .bin -s 0 -e 27901 -f f1113
