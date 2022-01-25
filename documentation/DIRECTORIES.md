## Directories
The repository contains the following directories

Directory             | Function
-------------         | -------------
cmake                 | cmake modules including third parties
documentation         | detailed documentation
jenkins               | Continuous Integration scripts
src *                 | the detector event processing pipeline (and unit tests)
utils                 | misc helper scripts and programs, notable efushell.py and efustats.py


# utils
Contain python scripts to query the command line interface of the event formation unit.
Of particular use are
* utils/efushell/efustats.py - reports detector-specific application counters
* utils/efushell/efustatus/py - reports application information (sw version, available commands, ...)
