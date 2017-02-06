#!/usr/bin/python

# Copyright (C) 2016, 2017 European Spallation Source ERIC

from __future__ import print_function
import sys
from numpy import genfromtxt
import pylab as pl
import matplotlib.pyplot as plt

if len(sys.argv) != 2:
   print("Usage ./projections.py runbasename")
   sys.exit(0)

file = sys.argv[1]

xydata = genfromtxt(file +'.events.xyproj', delimiter=' ')
zydata = genfromtxt(file +'.events.zyproj', delimiter=' ')
xzdata = genfromtxt(file +'.events.xzproj', delimiter=' ')

plt.suptitle(file)
plt.subplot(1,3,1)

plt.imshow(xydata, interpolation="none", extent = [1,8, 48, 1])
plt.colorbar()

plt.subplot(1,3,2)
plt.imshow(zydata, interpolation="none", extent = [1,16, 48, 1])
plt.colorbar()

plt.subplot(1,3,3)
plt.imshow(xzdata, interpolation="none", extent = [1,8, 16, 1])
plt.colorbar()

plt.tight_layout()
plt.show()
