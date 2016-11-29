#!/bin/bash

DIR="/nfs/groups/multigrid/data/raw/MG_CNCS/07_25/"

./analyse -d $DIR -p 2016_07_25_1051_sample_ -s 0 -e  1365 > 1051.csv
./analyse -d $DIR -p 2016_07_26_1005_sample_ -s 0 -e  1442 > 1005.csv
./analyse -d $DIR -p 2016_07_27_1113_sample_ -s 0 -e 27901 > 1113.csv
