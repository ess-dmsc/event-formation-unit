#!/usr/bin/python
# -*- coding: utf-8 -*-

# Copyright (C) 2016, 2017 European Spallation Source ERIC

from __future__ import print_function, absolute_import
from numpy import *
import vtk, sys

# A simple function to be called when the user decides to quit the application.
def exitCheck(obj, event):
    if obj.GetEventPending() != 0:
        obj.SetAbortRender(1)

# Alpha channels
def alphahandmade():
   af = vtk.vtkPiecewiseFunction()
   af.AddPoint(0, 0.0)
   af.AddPoint(50, 0.1)
   af.AddPoint(100, 0.2)
   af.AddPoint(150, 0.4)
   af.AddPoint(200, 0.9)
   af.AddPoint(255, 1.0)
   return af

def alphalinear(lowcutoff):
   af = vtk.vtkPiecewiseFunction()
   for value in range(255):
      if value >= lowcutoff:
         af.AddPoint(value, value/255.0)
   return af

# Colormaps
def greyscale():
   cf = vtk.vtkColorTransferFunction()
   for value in range(255):
      cf.AddRGBPoint(  value, value/255.0, value/255.0, value/255.0)
   return cf

def blackbody():
   cf = vtk.vtkColorTransferFunction()
   cf.AddRGBPoint(  0, 0.0, 0.0, 1.0)
   cf.AddRGBPoint( 64, 0.0, 1.0, 1.0)
   cf.AddRGBPoint(128, 0.0, 1.0, 0.0)
   cf.AddRGBPoint(192, 1.0, 1.0, 0.0)
   cf.AddRGBPoint(255, 1.0, 0.0, 0.0)
   return cf

def blackbodyII():
   cf = vtk.vtkColorTransferFunction()
   #cf.AddRGBPoint(  0, 1.0, 1.0, 1.0)
   cf.AddRGBPoint(  0, 0.0, 0.0, 0.0)
   cf.AddRGBPoint( 50, 0.0, 0.0, 1.0)
   cf.AddRGBPoint(100, 0.0, 1.0, 1.0)
   cf.AddRGBPoint(150, 0.0, 1.0, 0.0)
   cf.AddRGBPoint(200, 1.0, 1.0, 0.0)
   cf.AddRGBPoint(250, 1.0, 0.0, 0.0)
   return cf


if len(sys.argv) != 2:
   print("usage: voxelrender.py filename")
   sys.exit(0)

filename = sys.argv[1]

x = 16
y = 48
z = 8

# Read linear data from file, interpret as 3D matrix with X,Y,Z dimensions
# specified above
data_matrix = fromfile(filename, dtype = int32).reshape([z,y,x])*1.0
#data_matrix = log10(data_matrix)

# When viewed from the front, the image appears mirrored in the x-direction
# so we swap elements
data_matrix = data_matrix[::-1,:,:]
#data_matrix[0:16] = data_matrix.max() # line in the Z-direction from Voxel 1
#data_matrix[16:32] = data_matrix.max() # line in z dir. from Voxel 17
#data_matrix[1*48*16:1*48*16 + 16] = data_matrix.max() # z dir. second column

# Normalize to 255 (for RGB color scales and transparency)
data_matrix = uint8((data_matrix / data_matrix.max()) * 255.0)


# Import into VTK
dataImporter = vtk.vtkImageImport()
data_string = data_matrix.tostring()
dataImporter.CopyImportVoidPointer(data_string, len(data_string))

dataImporter.SetDataScalarTypeToUnsignedChar()

dataImporter.SetNumberOfScalarComponents(1)

dataImporter.SetDataExtent( 0, x - 1, 0, y - 1, 0, z - 1)
dataImporter.SetWholeExtent(0, x - 1, 0, y - 1, 0, z - 1)

# Select color and transparency mappings
#colorFunc = greyscale()
#colorFunc = blackbody()
colorFunc = blackbodyII()
#alphafunc = alphalinear(30)
alphafunc = alphahandmade()

# Dunno
volumeProperty = vtk.vtkVolumeProperty()
volumeProperty.SetColor(colorFunc)
volumeProperty.SetScalarOpacity(alphafunc)

# Dunno
compositeFunction = vtk.vtkVolumeRayCastCompositeFunction()

volumeMapper = vtk.vtkVolumeRayCastMapper()
volumeMapper.SetVolumeRayCastFunction(compositeFunction)
volumeMapper.SetInputConnection(dataImporter.GetOutputPort())


volume = vtk.vtkVolume()
volume.SetMapper(volumeMapper)
volume.SetProperty(volumeProperty)

# With almost everything else ready, its time to initialize the renderer and window, as well as creating a method for exiting the application
renderer = vtk.vtkRenderer()
renderWin = vtk.vtkRenderWindow()
renderWin.AddRenderer(renderer)
renderInteractor = vtk.vtkRenderWindowInteractor()
#renderInteractor.AddObserver('KeyPressEvent', key_handler)
renderInteractor.SetRenderWindow(renderWin)

# Display axis
transform = vtk.vtkTransform()
transform.Scale(3.0, 3.0, 3.0)

axes = vtk.vtkAxesActor()
axes.SetUserTransform(transform)
axes.SetXAxisLabelText("")
axes.SetYAxisLabelText("")
axes.SetZAxisLabelText("")
renderer.AddActor(axes)

txt = vtk.vtkTextActor()
txt.SetInput(filename)
txtprop=txt.GetTextProperty()
txtprop.SetFontFamilyToArial()
txtprop.SetFontSize(18)
txtprop.SetColor(0.5, 0.5, 0.5)
txt.SetDisplayPosition(10,1000)
renderer.AddActor(txt)

# We add the volume to the renderer ...
renderer.AddVolume(volume)

#renderer.SetBackground(1, 1, 1)  # white
renderer.SetBackground(0, 0, 0)  # black
renderWin.SetSize(1024, 1024)

ax2 = vtk.vtkOrientationMarkerWidget()
ax2.SetOrientationMarker(axes)
ax2.SetInteractor(renderInteractor)
ax2.EnabledOn()
ax2.InteractiveOn()
renderer.ResetCamera()

# Tell the application to use the function as an exit check.
renderWin.AddObserver("AbortCheckEvent", exitCheck)

renderInteractor.Initialize()
renderWin.Render()
renderInteractor.Start()
