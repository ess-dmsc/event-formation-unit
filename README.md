# Event Formation Unit

[![DOI](https://zenodo.org/badge/80731668.svg)](https://zenodo.org/badge/latestdoi/80731668)


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


# Details

## dataformats
Contains a heterogenous bunch of tools for various purposes. For example:
* lua script plugins for Wireshark for vmm and sonde data formats
* python scripts for visualising processed multigrid data
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
Contains detector pipeline plugins for a number of prototype detectors:
* Gd-GEM for the NMX instrument - gdgem.so
* Multiblade - mbcaen.so
* multigrid - mgcncs.so and mgcncs2.so
* SoNDe - sone.so

Also contains data-generators, some reading data from file other from Wireshark captures,
some generating constant data.
* mgcncsgen - generate constant data to grid 1 and wire 113 (front, top, left pixel)
* mgcncsgenfile - reads from (binary) file and sends data to EFU
* mgcncsgenjson - reads from file runs specified in json file
* gennmfpcap - reads data from Wireshark and sends it to EFU (not GEM specific)

## udp
udp is built automatically by prototype2.

Only used occasionally for basic udp rx and tx testing.

## Building
This project can be built either using Conan to provide the dependencies or by manually making sure that the dependencies are available. The instructions for both alternatives are listed below.

Note that regardless of the method used, a C++ compiler which supports C++11 is required.

### Using Conan
By using Conan, all dependencies are downloaded and compiled (if required) automatically. You will need to have the following installed:

* [**Conan**](https://conan.io) The conan script has to be available in the current ``$PATH``. Note also that this has only been tested with version 1.0.4 of conan.
* [**CMake**](https://cmake.org) At least CMake version 2.8.12 is required. Some packages that requires more recent versions of CMake will download this as a dependency.
* [**bash**](https://www.gnu.org/software/bash/) For properly setting paths to the conan provided dependencies.

For conan to know where the dependencies can be downloaded from, package repositories must be added by running the following commands:

* ``conan remote add ess-dmsc https://api.bintray.com/conan/ess-dmsc/conan``
* ``conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan``

Note also that for additional functionality you might want to install the following dependencies:

* [**libpcap**](http://www.tcpdump.org)
* [**Valgrind**](http://valgrind.org) For doing memory usage (and other) tests.
* **lcov/gcov/gcovr** Required to generate coverage reports.

```bash
cd event-formation-unit

mkdir build

cd build

cmake ..

make
```

### Installing dependencies manually
What follows is a list of dependencies that are required for building this project. Note that the version numbers listed are what has been tested. Other versions may work as well.

* [**CMake**](https://cmake.org) _Version 3.5.1_
* [**Google Test**](https://github.com/google/googletest) _Version 1.8.0_
* [**FlatBuffers**](https://github.com/google/flatbuffers) _Version 1.8.0_ If compiling from source, pass the argument ``-DCMAKE_BUILD_TYPE=Release`` to CMake in order to build and install the *flatc* application.
* [**librdkafka**](https://github.com/edenhill/librdkafka) _Version 0.11.3_
* [**Boost**](https://boost.org) _Version 1.58.0_
* [**HDF5**](https://support.hdfgroup.org/HDF5/) _Version 1.8.13_
* [**h5cpp**](https://github.com/ess-dmsc/h5cpp) _Version 0.0.5_
* [**Streaming Data Types**](https://github.com/ess-dmsc/streaming-data-types) Note that you have to run CMake for this project in order to generate the required header files. The directory including the header files will have to be given as an argument for running the EFU CMake file. See below for instructions on how to do that.
* [**ESSGeometry**](https://github.com/ess-dmsc/logical-geometry) The header file provided by this dependency (`ESSGeometry.h`) has to be manually moved to a directory named `logical_geometry`. The directory containing this directory then has to be passed to the EFU CMake file in the same way as done for the *Streaming Data Types* dependency. See below.
* [**CLI11**](https://github.com/CLIUtils/CLI11) As of this writing, the master branch is required.
* [**ASIO**](https://think-async.com/Asio) _Version 1.10.0_

Extra features are also provided by the following dependencies:
* [**libpcap**](http://www.tcpdump.org)
* [**TCLAP**](http://tclap.sourceforge.net)
* [**Valgrind**](http://valgrind.org)
* [**graylog-logger**](https://github.com/h5cpp/graylog-logger)

After having installed the dependencies, this code in this repository can be compiled. Note that when running *cmake*, you have to also set the path to the header files generated by the Streaming Data Types and logical geometry dependencies as illustrated in the following example:

```bash
cd event-formation-unit

mkdir build

cd build

cmake -DCMAKE_CXX_FLAGS="-I/some_path/streaming-data-types/build/schemas/;-I/path_that_contains_logical_geometry/" ..

make
```
You can also set the ``$CXXFLAGS``environment variable in order to minimise typing on repeated runs of CMake:

```bash
export CXXFLAGS="-I/some_path/streaming-data-types/build/schemas/"
```

## System independent stuff

### MacOSX
Building both with and without conan appears to be working on High Sierra (10.13.2).

### Centos 7
Using Conan to provide the dependencies appears to be working well under Centos 7.4. Building and/or installing the dependencies manually has not been tested under Centos.

### Ubuntu 16
Wen using conan to provide the dependencies, an extra option has to be provided: `--settings compiler.libcxx=libstdc++11`. Thus the call to conan turns into:

```bash
conan install --build=outdated .. --settings compiler.libcxx=libstdc++11
```

## Targets and options
See options and targets by

    > make hints

Target                | Description
-------------         | -------------
*hints*                 | DEFAULT and NORMATIVE target and option guide
*help*                  | lists all available make targets
*runtest*               | run test executables, generate xml reports
*coverage*              | generate test coverage report in html/index.html
*valgrind*              | run valgrind on tests, generate reports
