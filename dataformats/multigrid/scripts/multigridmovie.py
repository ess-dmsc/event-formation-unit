#!/usr/bin/python

# Copyright (C) 2016, 2017 European Spallation Source ERIC

from __future__ import print_function
import sys
import numpy as np
import pylab as pl
import matplotlib.pyplot as plt

class Proj:
    def __init__(self):
        self.x = 8
        self.y = 48
        self.z = 16
        self.clear()


    def coords(self, pixel):
        x = (pixel - 1) / (self.y * self.z)
        y = (pixel - 1) / self.z % self.y
        z = (pixel - 1) % self.z
        return [x,y,z]

    def addpixels(self, pixel):
        x,y,z = self.coords(pixel)

        #print("pixel: %d, x, y, z: %d, %d, %d\n" % (pixel, x, y, z))
        for i in range(len(x)):
            self.xy[y[i],x[i]] += 1
            self.zy[y[i],z[i]] += 1
            self.xz[self.z - z[i] - 1,x[i]] += 1

    def addpixel(self, pixel):
        x,y,z = self.coords(pixel)

        #print("pixel: %d, x, y, z: %d, %d, %d\n" % (pixel, x, y, z))

        self.xy[y,x] += 1
        self.zy[y,z] += 1
        self.xz[self.z - z - 1,x] += 1

    def clear(self):
        self.xy = np.zeros((self.y, self.x))
        self.zy = np.zeros((self.y, self.z))
        self.xz = np.zeros((self.z, self.x))

    def plot(self, title):
        pl.ion()
        pl.clf()

        plt.suptitle(title)
        plt.subplot(1,3,1)
        plt.title("x-y")
        plt.imshow(self.xy, interpolation="none")
        plt.colorbar()

        plt.subplot(1,3,2)
        plt.title("z-y")
        plt.imshow(self.zy, interpolation="none")
        plt.colorbar()

        plt.subplot(1,3,3)
        plt.title("x-z")
        plt.imshow(self.xz, interpolation="none")
        plt.colorbar()

        pl.pause(0.001)
        pl.savefig(title + ".png")


proj = Proj()
sampling = 100000

if len(sys.argv) == 2:
    file = sys.argv[1]
elif len(sys.argv) == 3:
    file = sys.argv[1]
    sampling = int(sys.argv[2])
else:
   print("Usage ./projections.py runbasename [samplesperplot]")
   sys.exit(0)

with open(file + ".events.coord") as f:
    sampcount = 0
    totcount = 0
    for line in f:
        if line[0] == '#':
            continue
        pix = int(line.split(',')[3])
        proj.addpixel(pix)
        sampcount += 1
        if sampcount == sampling:
            proj.plot("events_%d_to_%d" % (totcount, totcount + sampling))
            proj.clear()
            sampcount = 0
            totcount += sampling
    proj.plot("events_%d_to_%d" % (totcount, totcount + sampling))
