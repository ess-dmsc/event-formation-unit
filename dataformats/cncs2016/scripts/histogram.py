#!/usr/bin/env python

import numpy as np
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
import sys, csv
import future #pip install future

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

def printhist(values):
   valmax = max(values)
   H = np.zeros(valmax + 1)
   for i in values:
      H[i] += 1

   print(", ".join(str(int(x)) for x in H))


nmax = 1
tot = 0
dt = []
w0pos = []
w0amp = []
g0pos = []
g0amp = []
with open(file, 'rb') as csvfile:
  reader = csv.reader(csvfile, delimiter=',')
  for row in reader:
     dt.append(int(row[1]))
     w0amp.append(int(row[2]))
     w0pos.append(int(row[4]))
     g0amp.append(int(row[6]))
     g0pos.append(int(row[8]))
     nmax = nmax + 1
     if (nmax == N):
        plt.subplot(511)
        plt.title("events " + str(tot) + " to " + str(tot + nmax))
        #histogram("time", 1000, (100000, 150000), dt)
        histogram("time", 1000, None, dt)

        plt.subplot(512)
        histogram("w0amp", 1000, (min(w0amp), max(w0amp)), w0amp)

        plt.subplot(513)
        #printhist(w0pos)
        histogram("w0pos", 1000, (min(w0pos), max(w0pos)), w0pos)

        plt.subplot(514)
        histogram("g0amp", 1000, (min(g0amp), max(g0amp)), g0amp)

        plt.subplot(515)
        histogram("g0pos", 1000, (min(g0pos), max(g0pos)), g0pos)
        #histogram("g1pos", 1000, None, g1pos)
        plt.show()
        dt = []
        w1pos = []
        g1pos = []
        tot = tot + nmax
        nmax = 1
