## Directories
The repository contains the following directories

Directory             | Function
-------------         | -------------
cmake                 | cmake modules including third parties
documentation         | detailed documentation
jenkins               | Continuous Integration scripts
prototype2 *          | the detector event processing pipeline (and unit tests)
system-tests          | scripts for running the event formation unit as a system test
utils                 | misc helper scripts and programs, notable efushell.py and efustats.py

# Details

## prototype2
Contains detector pipeline plugins for a number of prototype detectors:
* Gd-GEM - NMX instrument
* Multi-Blade - Freia and Estia instruments
* Multi-Grid - CSPEC, TREX, VOR instruments
* SoNDe - SKADI

# utils
Contain python scripts to query the command line interface of the event formation unit.
Of particular use are
* utils/efushell/efustats.py - reports detector-specific application counters
* utils/efushell/efustatus/py - reports application information (sw version, available commands, ...)
