#!/usr/bin/python

import numpy as np
import matplotlib.pyplot as plt

print "loading"
x,y, z =np.loadtxt('tmp.dat.hist',delimiter=',',usecols=(0,1,2),unpack=True)
nx = x.max() - x.min() + 1
xmin = x.min()
ny = y.max() - y.min() + 1
Z = np.zeros((ny,nx))

print "x max: %d, min: %d\n" % (x.max(), x.min())
print "y max: %d, min: %d\n" % (y.max(), y.min())

print "create matrix"
print "range x", len(x)
assert x.shape == y.shape == z.shape

for i in range(len(x)):
    Z[y[i]][x[i]-xmin] = z[i]

print "plot"
fig = plt.figure()
figure_name = 'w1pos'
plt.pcolor(np.arange(nx),np.arange(ny),Z,cmap=plt.cm.hot)
plt.colorbar()
plt.xlim(0,x.max()-x.min())
plt.ylim(0,y.max()-y.min())
plt.show()
