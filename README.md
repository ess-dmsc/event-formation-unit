# Code Snippets and Experiments
This repository holds code snippets in C++, C and bash for testing ideas
 related to network packet processing and  event processing.

## Directories
The repository contains the following directories

Directory             | Function
-------------         | -------------
centos                | scripts to configure link bonding on CentOS
cmake_modules         | cmake modules from third parties
dataformats **        | sample detector data and data parsers (and unit tests)
jenkins               | Continuous Integration scripts
libs *                | utility classes and unit tests
monitors              | schemas and python scripts for testing Kafka based monitors
multicore_iperf_tests | testing tcp and udp performance across local cores (obsolete)
prototype2 *          | the detector event processing pipeline (and unit tests)
udp *                 | udp receive tests (bsd socket, probably obsolete)
utils                 | misc helper scripts and programs

## Build using Conan to supply dependencies

### Required conan repositories

This project uses the following conan repositories to download the required dependencies:

* https://api.bintray.com/conan/ess-dmsc/conan
* https://api.bintray.com/conan/conan-community/conan
* https://api.bintray.com/conan/bincrafters/public-conan

To add a conan repository, use the following command:

```
conan remote add <local-name> <remote-url>
```

### Download dependencies and build project

```
cd event-formation-unit
mkdir build
cd build
conan install ../ --build-missing
cmake ..
make
````


# Details

## dataformats
See README in subdir for build instruction.

Contains a heterogenous bunch of tools for various purposes. For example:
* lua script plugins for Wireshark for vmm and sonde data formats
* python scripts for visualizing processed multigrid data
* python scripts for testing implementation of logical geometry for multigrid and sonde
* C++ code for batch processing of multigrid data

## libs
libs are built automatically by prototype2.

Library functions that can be used by other projects such as
* BSD socket wrapper functions
* Posix tread wrapper with thread affinity
* us and cpu clock (TSC) based timers
* Single producer single consumer FIFO

## prototype2
See README in subdir for build instructions.

Contains detector pipeline plugins for a number of prototype detectors:
* Gd-GEM for the NMX instrument - gdgem.so
* Multiblade - mbcaen.so
* multigrid - mgcncs.so and mgcncs2.so
* SoNDe - sone.so

Also contains datagenerators, some reading data from file other from Wireshark captures,
some generating constant data.
* mgcncsgen - generate constant data to grid 1 and wire 113 (front, top, left pixel)
* mgcncsgenfile - reads from (binary) file and sends data to EFU
* mgcncsgenjson - reads from file runs specified in json file
* gennmfpcap - reads data from WIreshark and sends it to EFU (not GEM specific)

## udp
udp is built automatically by prototype2.

Only used occasionally for basic udp rx and tx testing.
