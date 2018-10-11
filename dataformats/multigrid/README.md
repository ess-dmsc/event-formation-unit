#About

This project is written for analysing raw multigrid data as read out by
(some readout system) in experiments done in 2016 (and probably 2017).

## programs and scripts

The following scripts are available

 * scripts/heatmap.py
 * scripts/histogram.py
 * scripts/multiplot.py
 * scripts/voxelrender.py

In addition there are other, less significant, bash scripts and python scripts which have
been used during development.

## Visualization scripts

### Voxel renderer
Displays a 3D image of the detector based on the output.vox voxel intensity
file. The script uses vtk whose Python bindings must be installed.

    > ./voxelrender.py output.voxel

### Histograms
Displays a histogram of time of flight values, wire0 amplitudes and positions and
grid0 amplitudes and positions.

    > ./histogram.py output.events number_of_events

### Multiplot
Reads a csv data file and generates a plot of one or more columns which is
saved as a png file. Now takes several arguments to specify its behavior
(see multiplot -h).

    > ./multiplot.py -t MyTitle -c "2 1 7" file1.csv

Plots columns 2, 1 and 7 in a single figure.

    > ./multiplot.py -b -o myfile.png -c "2 1 7" file1.csv

Plots columns 2, 1 and 7 in a single figure. Saves to myfile.png but does not pause
to display the plot (batch mode).

    > ./multiplot.py file1.csv file2.csv file3.csv

Plots columns 1 from each file in a separate graph.

### Heatmap
Generates a heatmap of w0 positions as a function of file number.

    > ./heatmap.py output.hist
