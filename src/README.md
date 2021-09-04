
## Directories

Source files for efu, detector modules and data generators.

Directory             | Function
-------------         | -------------
common                | common detector code
efu                   | EFU main application code
modules               | detector implementations
test                  | unit test utilities:
udpgenpcap            | replay of network (pcap) traffic


## modules
Contains detector pipeline plugins for a number of prototype detectors and monitors:
### Instrument pipelines
* loki - LoKI instrument
* freia - Freia instrument
* adc_readout - monitor

### Prototypes
* dream - DREAM instrument (simulation)
* gdgem - NMX instrument (srs)
* multiblade - Freia and Estia instruments (caen)
* multigrid - CSPEC instrument (mesytec)
* sonde - SKADI prototype (ideas)

### Utilities
* readout - ESS/VMM3 readout
* generators - UDP data generators for various sources
* perfgen - generator of events for benchmarking



## external modules
It is possible to build modules with sources outside the efu repository. To
do this we make use of EFU_EXTERNAL_DIR which is a cmake variable containing
a list of directories with cmake modules to be included.

    > cmake -D EFU_EXTERNAL_DIR= "/tmp/thisrepo;/tmp/thatrepo" ..
