#!/usr/bin/env python

import numpy as np
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
import csv


def histogram(text, bins, values):
   n, bins, patches = plt.hist(values, bins, range=(140, 4000), normed=0, facecolor='green', alpha=0.75)

   plt.xlabel(text)
   plt.ylabel('Count')
   plt.title(text)
   #plt.axis([40, 160, 0, 0.03])
   plt.grid(True)

   plt.show()

N=1000000
max = 1
dt = []
w1pos = []
with open('1113.dat', 'rb') as csvfile:
  reader = csv.reader(csvfile, delimiter=',')
  for row in reader:
     #print row
     #dt.append(int(row[1]))
     w1pos.append(int(row[4]))
     #print row[1]
     max = max + 1
     if (max == N):
        subplot(211)
        histogram("time", 50, dt)
        subplot(212)
        histogram("w1pos", 100, w1pos)
        dt = []
        w1pos = []
        max = 1
