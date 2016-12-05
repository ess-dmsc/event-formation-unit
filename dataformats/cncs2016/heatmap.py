#!/usr/bin/python

# Copyright (C) 2016 European Spallation Source ERIC

import numpy as np
import matplotlib.pyplot as plt

print "loading..."
x,y, z =np.loadtxt('f1113.hist',delimiter=',',usecols=(0,1,2),unpack=True)
nx = x.max() - x.min() + 1
xmin = x.min()
ny = y.max() + 1
Z = np.zeros((ny,nx))

print "creating matrix..."
assert x.shape == y.shape == z.shape

for i in range(len(x)):
    Z[int(y[i])][int(x[i]-xmin)] = z[i]

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
