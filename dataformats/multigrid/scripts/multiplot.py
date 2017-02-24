#!/usr/bin/python

# Copyright (C) 2016, 2017 European Spallation Source ERIC

from __future__ import print_function
import sys, pylab as pl, numpy as np
import getopt

def usage():
    print("usage: multiplot.py [options] files")
    print("options")
    print( " -b, --batch                  does not pause for plotting")
    print(" -c, --columns columstr       specify the number of columns to simultaneously plot")
    print(" -t, --title textstr          provide a title for the plot")
    print(" -o, --output files           name for saved png file")
    print(" -h, --help                   prints this text")
    print("")
    print(" Plots columns of the .csv files.")
    return

try:
    opts, args = getopt.getopt(sys.argv[1:], "hbc:t:o:", ["help", "title=", "columns=", "output="])
except getopt.GetoptError as err:
    print(err)
    usage()
    sys.exit(2)

batch = 0
title = "Multiplot"
columns = "1"
pngfile="multiplot.png"

for opt, arg in opts:
    if opt in ("-h", "--help"):
        usage()
        sys.exit()
    elif opt in ("-c", "--columns"):
        columns = arg
    elif opt in ("-o", "--output"):
        pngfile = arg
    elif opt in ("-b", "--batch"):
        batch = 1
    elif opt in ("-t", "--title"):
        title = arg

files = args
n = len(files)

if n == 0:
    print("Error: no files specified")
    usage()
    sys.exit(2)

nc, nr = [1, n]
if (n > 2):
    nc = int(pl.ceil(pl.sqrt(n)))
    nr = int(pl.ceil(1.0 * n / nc))

pl.rcParams.update({'font.size': 8})
pl.title(title)
for idx, file in enumerate(files):
   pl.subplot(nr, nc, idx + 1)
   data = np.loadtxt(file, delimiter=',', usecols = (1,2,3,4,5,6,7,8,9,10,11) )
   x = data[:,0]
   for yi in columns.split():
     y = data[:,int(yi)]
     pl.plot(x,y, lw = 0.5)

pl.legend(columns.split())

pl.savefig(pngfile)
if not batch:
    pl.show()
