# Dataformats


## Programs
The following programs are built

Executable            | Function
-------------         | -------------
multigrid/batchreader | Processes multiple testruns of CNCS data for multigrid
multigrid/genpixids   | Generates pixel ids from an eventfile and a calibration file. <br> These are created by batchreader
multigrid/cncsread    | Obsolete - an old parser for multigrid readout data

Some unit tests are also built.

## Build instructions

To build:

    > cd event-formation-unit/dataformats
    > mkdir build
    > cd build
    > cmake ..
    > make
