## Directories
The repository contains the following directories

Directory             | Function
-------------         | -------------
cmake                 | cmake modules including third parties
documentation         | detailed documentation
jenkins               | Continuous Integration scripts
libs *                | utility classes and unit tests
prototype2 *          | the detector event processing pipeline (and unit tests)
utils                 | misc helper scripts and programs, notable efushell.py and efustats.py

# Details

## libs
libs are built automatically by prototype2.

Library functions that can be used by other projects such as
* BSD socket wrapper functions
* Posix tread wrapper with thread affinity
* us and cpu clock (TSC) based timers
* Single producer single consumer FIFO

## prototype2
Contains detector pipeline plugins for a number of prototype detectors:
* Gd-GEM - NMX instrument
* Multi-Blade - Freia and Estia instruments
* Multi-Grid - CSPEC, TREX, VOR instruments
* SoNDe - 

Also contains data-generators, some reading data from file other from Wireshark captures,
some generating constant data.
* mgcncsgen - generate constant data to grid 1 and wire 113 (front, top, left pixel)
* mgcncsgenfile - reads from (binary) file and sends data to EFU
* mgcncsgenjson - reads from file runs specified in json file
* gennmfpcap - reads data from Wireshark and sends it to EFU (not GEM specific)
