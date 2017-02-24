#!/usr/bin/python

# Copyright (C) 2016, 2017 European Spallation Source ERIC

from __future__ import print_function, absolute_import

class MultiGridGeometry:

   def __init__(self, panels, modules, grids, nx, nz):
      self.P = panels
      self.M = modules
      self.G = grids
      self.NX = nx
      self.NZ = nz

   def Voxel(self, x,y,z):
      return (x - 1) * self.G * self.NZ + (y - 1) * self.NZ + z

   def XYZ(self, voxel):
      x = (voxel - 1) / (self.G * self.NZ) + 1
      y = (voxel - 1) / self.NZ % self.G + 1
      z = (voxel - 1) % 16 + 1
      return [x, y, z]

def xyz2voxel(mg, x, y, z):
   print("Coordinates (%3d, %3d, %3d)  ->  Voxel %4d" % (x,y,z, mg.Voxel(x,y,z)))

def voxel2xyz(mg, voxel):
   print("Voxel %4d  ->  coordinates (%3d, %3d, %3d)" % (voxel, mg.XYZ(voxel)[0], mg.XYZ(voxel)[1], mg.XYZ(voxel)[2]))


MG = MultiGridGeometry(1, 2, 48, 4, 16)


xyz2voxel(MG, 1,1,16)
xyz2voxel(MG, 4,1,16)
xyz2voxel(MG, 1,1,1)
xyz2voxel(MG, 4,1,1)

print("")
xyz2voxel(MG, 1,48,16)
xyz2voxel(MG, 4,48,16)
xyz2voxel(MG, 1,48,1)
xyz2voxel(MG, 4,48,1)

print("")
xyz2voxel(MG, 8,1,16)
xyz2voxel(MG, 5,1,16)
xyz2voxel(MG, 8,1,1)
xyz2voxel(MG, 5,1,1)

print("")
xyz2voxel(MG, 8,48,16)
xyz2voxel(MG, 5,48,16)
xyz2voxel(MG, 8,48,1)
xyz2voxel(MG, 5,48,1)
