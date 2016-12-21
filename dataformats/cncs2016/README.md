#About

This project is written for analysing raw multigrid data as readout by
(some readout system) in experiments done in 2016 (and probably 2017).

## programs and scripts

The following programs are built

 * cncsread
 * batchreader
 * genpixids

The following scripts are available

 * scripts/heatmap.py
 * scripts/histogram.py
 * scripts/multiplot.py
 * scripts/voxelrender.py

In addition there are some less significant  bash scripts and python scripts which have 
been used during development.

## Build and Run

To build the programs

`> make`


## Program description

### batchreader
THis is the workhorse of this project. It reads multiple files and generates event and other 
data which can then be used by other utilities to visualize the dataset.

`batchreader` currently runs in two modes

 * interactve batch mode where the user specifies a single 'run' to be analysed
 * filelist batch mode where multiple runs are analysed sequentially

`batchreader` takes several arguments `-h` produces a help text

`interactive batch mode example:
> ./batchmode -d /home/morten/data -p 2016_07_13_beamOn_4p96A_ -o .bin -s 1 -e 1000 -f filename -c cfilename
`
This will parse 1001 files with names 2016_07_13_beamOn_4p96A_0005.bin to 2016_07_13_beamOn_4p96A_1000.bin. The 
output files wil be named

`filename.events
filename.csv
filename.hist`

and the calibration files are named 

`cfilename.wcal
cfilename.gcal`

filelist mode example:
`> ./batchmode -r -s 7 -e 10 `
This will analyse runs 7 to 10, both inclusive. The definition of runs are in src/RawDataFiles.h. Output
filenames are generated automatically and corresponds the the filename prefix for the run.

### genpixids

This program takes two inputs: an event file and a calibration file prefix. It then reads the 
event file and generates a binary file of voxel intensities. Typically something like this:

`
> ./genpixids  2016_07_13_beamOn_4p96A_.events 2016_07_13_beamOn_4p96A_
`

a binary .vox file is generated.

### cncsread (obsolete)
This utility is an (mostly) idenpendent parser of the multi grid data format. It can be used 
to parse a single raw data file and will print out some basic statistics about the number of 
valid events etc.

`> ./cncsread 2016_07_13_beamOn_4p96A_135.bin`

## Visualization scripts


