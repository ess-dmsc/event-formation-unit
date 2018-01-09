#!/usr/bin/python

# Copyright (C) 2016, 2017 European Spallation Source ERIC

from __future__ import print_function
import numpy as np
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
import sys, csv

if len(sys.argv) != 3:
   print("Usage ./histogram.py eventfilename samples")
   sys.exit(0)

file = sys.argv[1]
N = int(sys.argv[2])

def histogram(text, bins, rng, values):
   n, bins, patches = plt.hist(values, bins, range=rng, normed=0, log=True, facecolor='green', alpha=0.75)
   #plt.xlabel(text)
   plt.ylabel(text + ' count')
   #plt.title(text)
   #plt.axis([40, 160, 0, 0.03])
   plt.grid(True)

nmax = 1
tot = 0
amp = []
with open(file, 'rb') as csvfile:
  reader = csv.reader(csvfile, delimiter=',')
  for row in reader:
     amp.append(int(row[3]))
     nmax = nmax + 1
     if (nmax == N):
        plt.title("Mesytec readout for Multigrid")
        plt.subplot(111)
        histogram("amp", 256, (0, 1000), amp)
        # histogram("amp", 4096, (0, max(amp)), amp)
        # histogram("amp", 4096, None, amp)
        plt.show()
        amp = []
        tot = tot + nmax
        nmax = 1
