# Gd-GEM Detector event processing


Files/Directories     | Function
-------------         | -------------
nmx/                  | Contains common code for event processing
nmxgen/               | Data generators for Gd-GEM: gennmxfile, gennmxpcap
vmm2srs/              | Code for parsing VMM data by the SRS readout system
gdgem.cpp             | Plugin for the Gd-GEM pipeline
nmx_config.json       | config file for NMX, can be read by -f option to the EFU


## Starting the Detector
The SRS readout system sends data on UDP port 6006, whereas the efu2 has a
default value of 9000.

To 'lock' threads to specific cores use -c lcore_id. If you dont
care use -c -5.

To start gdgem

`> ./efu2 -d gdgem -f nmx_config.json -p 6006 -c -5`
