#!/usr/bin/env python

import numpy as np
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
import csv


def histogram(text, bins, rng, values):
   n, bins, patches = plt.hist(values, bins, range=rng, normed=0, facecolor='green', alpha=0.75)

   plt.xlabel(text)
   plt.ylabel('Count')
   plt.title(text)
   #plt.axis([40, 160, 0, 0.03])
   plt.grid(True)


N=1000000
max = 1
dt = []
w1pos = []
with open('1113.dat', 'rb') as csvfile:
  reader = csv.reader(csvfile, delimiter=',')
  for row in reader:
     dt.append(int(row[1]))
     w1pos.append(int(row[4]))
     max = max + 1
     if (max == N):
        plt.subplot(211)
        histogram("time", 50, None, dt)
        plt.subplot(212)
        histogram("w1pos", 100, (150, 4000), w1pos)
        plt.show()
        dt = []
        w1pos = []
        max = 1
