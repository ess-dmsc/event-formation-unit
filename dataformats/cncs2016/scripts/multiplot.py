#!/usr/bin/python

import sys, pylab as pl, numpy as np

if len(sys.argv) != 4:
    print "usage: multiplot.py title columns files"
    sys.exit(0)

title = sys.argv[1]
cols = sys.argv[2]
files = sys.argv[3:]
n = len(files)

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
   for yi in cols.split():
     y = data[:,int(yi)]
     pl.plot(x,y, lw = 0.5)

pl.legend(cols.split())
pl.savefig('test.png')
pl.show()
