# Multigrid detector event processing


Files/Directories     | Function
-------------         | -------------
calib_data/           | MG.CNCS specific calibration files for unit testing
mgcncs/               | Code for parsing mc.cncs readout data
generators/           | Data generators: mgcncsgen, mgcncsgenfile, mgcncsgenjson
mgcncs.cpp            | Obsolete - old three-stage pipeline
mgcncs2.cpp           | Plugin for the multigrid detector pipeline

## Starting the Detector
For MG.CNCS two steps must be completed before the detector is readout to process
readout data correctly
 * First the detector must be started
 * Then a calibration file matching the readout data must be loaded

To 'lock' threads to specific cores use -c lcore_id. If you dont
care use -c -5.

### Starting the plugin

`> ./efu -d mgcncs2 -c -5`

### Load a calibration file
Calibration files comes in pairs, one for the grids and one for the wires. These
have different suffixes, but only the base name should be specified.

A utility __mgloadcal.py__ has been created for doing this. It connects to the EFU
on the local host ip address and a default port number for the control shell. These
can be changed using -i and -p.

`> utils/efushell/mgloadcal.py /path/to/calibrationfile`
