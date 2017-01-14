#!/bin/bash

CSVS="../output/nfsoffline_01"
#CSVS="1051"

function plot()
{
   ofile=$1
   title=$2
   cols=$3
   file=$4
   ./multiplot.py -o $ofile -t "$title" -c "$cols" $file
}



for csv in $CSVS
do
   ifile=$csv".csv"
   echo "ifile: "$ifile
   plot $csv"_readout.png"        "readouts (blue), discarded (green)"       "1 2"   "$ifile"
   plot $csv"_events.png"         "valid events"                             "3"     "$ifile"
   plot $csv"_events_total.png"   "accumulated events"                       "4"     "$ifile"
   plot $csv"_nonzero.png"        "nonzero values (blue), globally (green)"  "5 8"   "$ifile"
   plot $csv"_firstnz.png"        "first nonzero (blue), globally (green)"   "6 9"   "$ifile"
   plot $csv"_lastnz.png"         "last nonzero (blue), globally (green)"    "7 10"  "$ifile"
done
