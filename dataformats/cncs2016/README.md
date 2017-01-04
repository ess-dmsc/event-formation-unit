#About

This project is written for analysing raw multigrid data as read out by
(some readout system) in experiments done in 2016 (and probably 2017).

## programs and scripts

The following programs are provided

 * batchreader
 * genpixids
 * cncsread

 To build the programs type

 `> make`

The following scripts are available

 * scripts/heatmap.py
 * scripts/histogram.py
 * scripts/multiplot.py
 * scripts/voxelrender.py

In addition there are other, less significant, bash scripts and python scripts which have
been used during development.

## Program description

### batchreader
This is the workhorse of this project. It reads multiple files and generates event and other
data which can then be used by other utilities to visualize the dataset. `batchreader` uses
the same classes as the prototype2/ project.

`batchreader` currently runs in two modes

 * **interactve batch mode** where the user specifies a single 'run' to be analysed
 * **filelist batch mode** where multiple runs are analysed sequentially

`batchreader` takes several arguments `-h` produces a help text

interactive batch mode example:

    > ./batchmode -d /home/morten/data -p 2016_07_13_beamOn_4p96A_ -o .bin -s 1 -e 1000 -f filename -c cfilename

This will parse 1001 files with names 2016_07_13_beamOn_4p96A_000.bin to 2016_07_13_beamOn_4p96A_1000.bin. The
output files wil be named

    filename.events
    filename.csv
    filename.hist

and the calibration files are named

    cfilename.wcal
    cfilename.gcal

filelist mode example:

    > ./batchmode -r runsfile.json -j runsobject -s 7 -e 10

This will analyse runs 7 to 10, both inclusive. The definition of runs are in the 
runsobject  of runsfile.json. Output filenames are generated automatically and corresponds 
the filename prefix or the individual runs.

### genpixids

This program takes either one or three inputs depending on whether event file and calibration filename
prefixes are identical or different. It then reads the event file and generates a binary file of voxel 
intensities. Typically something like this:

    > ./genpixids  runfile.events calibrationfile outputfile

or if runfile.events, runfile.wcal and runfile.gcal all exist

    > ./genpixids runfile 


a binary .vox file is generated. Either with the filename prefix from the second example or with the 
specified output filename from the first one.

### cncsread (obsolete)
This utility is an (mostly) idenpendent parser of the multi grid data format. It can be used
to parse a single raw data file and will print out some basic statistics about the number of
valid events etc.

`> ./cncsread 2016_07_13_beamOn_4p96A_135.bin`

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
saved as a png file.

    > ./multiplot.py "2 1 7" output.csv

Plots columns 2, 1 and 7 in a single figure.

### Heatmap
Generates a heatmap of w0 positions as a function of file number.

    > ./heatmap.py output.hist
