#!/usr/bin/python

# Copyright (C) 2016 European Spallation Source ERIC

import numpy as np
import matplotlib.pyplot as plt
import sys

if len(sys.argv) != 2:
   print "Usage ./heatmap.py xyzfile"
   sys.exit(0)

file = sys.argv[1]

print "loading..."
x, y, z =np.loadtxt(file, delimiter=',', usecols=(0,1,2), unpack=True)
xmin = x.min()
#ymin = y.min()
ymin = 0
nx = x.max() - xmin + 1
ny = y.max() - ymin + 1


Z = np.zeros((ny,nx))

print "creating matrix..."
assert x.shape == y.shape == z.shape

for i in range(len(x)):
    Z[ int(y[i]-ymin) ][ int(x[i]-xmin) ] = z[i]

print "plotting..."
fig = plt.figure()
ax = plt.subplot(111)
plt.title('w1pos - files ' + str(x.min()) + ' to ' + str(x.max()))
#plt.pcolor(np.arange(nx),np.arange(ny),Z,cmap=plt.cm.hot)
#
mesh = ax.pcolorfast(np.arange(nx), np.arange(ny), Z, cmap='gist_stern', vmin=0, vmax=np.abs(Z).max())
fig.colorbar(mesh)
plt.xlim(0,x.max()-x.min())
plt.ylim(0,y.max()-y.min())
plt.show()
