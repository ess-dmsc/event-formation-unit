#!/usr/bin/env python

import numpy as np
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
import sys, csv

if len(sys.argv) != 2:
   print "Usage ./histogram.py eventfilename"
   sys.exit(0)

file = sys.argv[1]

def histogram(text, bins, rng, values):
   n, bins, patches = plt.hist(values, bins, range=rng, normed=0, facecolor='green', alpha=0.75)
   #plt.xlabel(text)
   plt.ylabel(text + ' count')
   #plt.title(text)
   #plt.axis([40, 160, 0, 0.03])
   plt.grid(True)

def printhist(values):
   valmax = max(values)
   H = np.zeros(valmax)
   for i in range(valmax):
      H[values[i]] += 1

   print(", ".join(str(int(x)) for x in H))





N=1000000
nmax = 1
tot = 0
dt = []
w1pos = []
g1pos = []
with open(file, 'rb') as csvfile:
  reader = csv.reader(csvfile, delimiter=',')
  for row in reader:
     dt.append(int(row[1]))
     w1pos.append(int(row[4]))
     g1pos.append(int(row[8]))
     nmax = nmax + 1
     if (nmax == N):
        plt.subplot(311)
        plt.title("events " + str(tot) + " to " + str(tot + nmax))
        histogram("time", 1000, (120000, 145000), dt)
        plt.subplot(312)
        printhist(w1pos)
        histogram("w1pos", 1000, (250, 2500), w1pos)
        plt.subplot(313)
        histogram("g1pos", 1000, (250, 2000), g1pos)
        plt.show()
        dt = []
        w1pos = []
        g1pos = []
        tot = tot + nmax
        nmax = 1
