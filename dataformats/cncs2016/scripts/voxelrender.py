#!/usr/bin/env python
# -*- coding: utf-8 -*-

import vtk, sys
from numpy import *

if len(sys.argv) != 2:
   print "usage: voxelrender.py filename"
   sys.exit(0)

filename = sys.argv[1]

x = 8
y = 48
z = 16

data_matrix = fromfile(filename, dtype = int32).reshape([z, y, x]) * 1.0
data_matrix = uint8((data_matrix / data_matrix.max()) * 255.0)

dataImporter = vtk.vtkImageImport()

data_string = data_matrix.tostring()
dataImporter.CopyImportVoidPointer(data_string, len(data_string))

dataImporter.SetDataScalarTypeToUnsignedChar()

dataImporter.SetNumberOfScalarComponents(1)

dataImporter.SetDataExtent( 0, x - 1, 0, y - 1, 0, z - 1)
dataImporter.SetWholeExtent(0, x - 1, 0, y - 1, 0, z - 1)

# The following class is used to store transparencyv-values for later retrival. In our case, we want the value 0 to be
# completly opaque whereas the three different cubes are given different transperancy-values to show how it works.
alphaChannelFunc = vtk.vtkPiecewiseFunction()
alphaChannelFunc.AddPoint(0, 0.0)
alphaChannelFunc.AddPoint(1, 1.0)
alphaChannelFunc.AddPoint(2, 0.0)
alphaChannelFunc.AddPoint(130, 0.3)
alphaChannelFunc.AddPoint(180, 0.5)
alphaChannelFunc.AddPoint(240, 1.0)
alphaChannelFunc.AddPoint(255, 1.0)

# This class stores color data and can create color tables from a few color points. For this demo, we want the three cubes
# to be of the colors red green and blue.
colorFunc = vtk.vtkColorTransferFunction()
colorFunc.AddRGBPoint(  0, 1.0, 1.0, 1.0)
colorFunc.AddRGBPoint(  1, 0.0, 0.0, 0.0)
colorFunc.AddRGBPoint( 20, 0.0, 0.0, 1.0)
colorFunc.AddRGBPoint( 50, 0.0, 1.0, 1.0)
colorFunc.AddRGBPoint(130, 0.0, 1.0, 0.0)
colorFunc.AddRGBPoint(180, 1.0, 1.0, 0.0)
colorFunc.AddRGBPoint(255, 1.0, 0.0, 0.0)

# The preavius two classes stored properties. Because we want to apply these properties to the volume we want to render,
# we have to store them in a class that stores volume prpoperties.
volumeProperty = vtk.vtkVolumeProperty()
volumeProperty.SetColor(colorFunc)
volumeProperty.SetScalarOpacity(alphaChannelFunc)

# This class describes how the volume is rendered (through ray tracing).
compositeFunction = vtk.vtkVolumeRayCastCompositeFunction()
# We can finally create our volume. We also have to specify the data for it, as well as how the data will be rendered.
volumeMapper = vtk.vtkVolumeRayCastMapper()
volumeMapper.SetVolumeRayCastFunction(compositeFunction)
volumeMapper.SetInputConnection(dataImporter.GetOutputPort())

# The class vtkVolume is used to pair the preaviusly declared volume as well as the properties to be used when rendering that volume.
volume = vtk.vtkVolume()
volume.SetMapper(volumeMapper)
volume.SetProperty(volumeProperty)

# With almost everything else ready, its time to initialize the renderer and window, as well as creating a method for exiting the application
renderer = vtk.vtkRenderer()
renderWin = vtk.vtkRenderWindow()
renderWin.AddRenderer(renderer)
renderInteractor = vtk.vtkRenderWindowInteractor()
renderInteractor.SetRenderWindow(renderWin)

transform = vtk.vtkTransform()
#transform.Translate(-1.0, 0.0, 0.0)
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
txtprop.SetColor(0.0, 0.0, 0.0)
txt.SetDisplayPosition(10,1000)
renderer.AddActor(txt)

# We add the volume to the renderer ...
renderer.AddVolume(volume)
# ... set background color to white ...
renderer.SetBackground(1, 1, 1)
# ... and set window size.
renderWin.SetSize(1024, 1024)

ax2 = vtk.vtkOrientationMarkerWidget()
ax2.SetOrientationMarker(axes)
ax2.SetInteractor(renderInteractor)
ax2.EnabledOn()
ax2.InteractiveOn()
renderer.ResetCamera()

# A simple function to be called when the user decides to quit the application.
def exitCheck(obj, event):
    if obj.GetEventPending() != 0:
        obj.SetAbortRender(1)

# Tell the application to use the function as an exit check.
renderWin.AddObserver("AbortCheckEvent", exitCheck)

renderInteractor.Initialize()
# Because nothing will be rendered without any input, we order the first render manually before control is handed over to the main-loop.
renderWin.Render()
renderInteractor.Start()
