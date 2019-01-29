# Event Formation Unit
[![License (2-Clause BSD)](https://img.shields.io/badge/license-BSD%202--Clause-blue.svg)](https://github.com/ess-dmsc/event-formation-unit/blob/master/LICENSE) [![DOI](https://zenodo.org/badge/80731668.svg)](https://zenodo.org/badge/latestdoi/80731668) [![Build Status](https://jenkins.esss.dk/dm/job/ess-dmsc/job/event-formation-unit/job/master/badge/icon)](https://jenkins.esss.dk/dm/job/ess-dmsc/job/event-formation-unit/job/master/)

This project implements processing of neutron detector event data into neturon events. Pipelines for processing of raw data from Gd-GEM, Muli-Grid, Multi-Blade, SoNDe as well as a few other detectors have been implemented. Mostly implemented in C/C++.

A description of the contents of each directory in the root of the repository can be found in [documentation/DIRECTORIES.md](documentation/DIRECTORIES.md).

## Getting started

The [essdaq repository](https://github.com/ess-dmsc/essdaq) has scripts for automatically downloading and compiling this project. Instructions for manually compiling the event formation unit software follows.

### Prerequisites

To build and run this software the following dependencies are required.

* [**Conan**](https://conan.io) The conan script has to be available in the current ``$PATH``.
* [**CMake**](https://cmake.org) At least CMake version 2.8.12 is required. Some packages that requires more recent versions of CMake will download this as a dependency.
* [**bash**](https://www.gnu.org/software/bash/) For properly setting paths to the conan provided dependencies.
* A recent C/C++ compiler with support for C++14.

Conan is used to download dependencies. For conan to know where the dependencies can be downloaded from, package repositories must be added by running the following commands:

* `conan remote add conancommunity https://api.bintray.com/conan/conan-community/conan`
* `conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan`
* `conan remote add conan-transit https://api.bintray.com/conan/conan/conan-transit`
* `conan remote add ess-dmsc https://api.bintray.com/conan/ess-dmsc/conan

Note also that for additional functionality you might want to install the following dependencies manually:

* [**libpcap**](http://www.tcpdump.org)
* [**Valgrind**](http://valgrind.org) For doing memory usage (and other) tests.
* **lcov/gcov/gcovr** Required to generate coverage reports.

### Building

Run the following commands:

```bash
git clone https://github.com/ess-dmsc/event-formation-unit.git

cd event-formation-unit

mkdir build

cd build

cmake ..

make
```

#### Building under Ubuntu 16
Wen using conan to provide the dependencies, an extra option has to be provided: `--settings compiler.libcxx=libstdc++11`. Thus the call to conan turns into:

```bash
conan install --build=outdated .. --settings compiler.libcxx=libstdc++11
```

## Running the tests

### Unit tests
To run the unit tests for this project, run the following commands:

```
make runtest
```

### Other tests

It is also possible to get a test coverage report if the required prerequisites have been installed. To get the coverage report, run:

```
make coverage
```

To run a memory leak test (using Valgrind), run:

```
make valgrind
```

### [Running System tests (link)](https://github.com/ess-dmsc/event-formation-unit/blob/master/system-tests/README.md)

## Running the event formation application

En example of the commands required to run an event formation pipeline (in this case the *mbcaen* pipeline) follows:

```
make efu mbcaen
cd bin
./efu -d ../modules/mbcaen -f configfile --nohwcheck
```

Note you will need to provide a config file in the case of the *mbcaen* module as well.

To get the available command line arguments, use `-h` or `--help`. This works when providing a detector module argument as well. For example:
```
./efu -d ../modules/mbcaen -h
```

## Contributing

Please read the [CONTRIBUTING.md](CONTRIBUTING.md) file for details on our code of conduct and the process for submitting pull requests to us.

## Authors

* Morten Jagd Christensen
* Martin Shetty
* Jonas Nilsson

See also the [list of contributors](https://github.com/ess-dmsc/event-formation-unit/graphs/contributors) on Github.

## License

This project is licensed under the BSD-2 License see the [LICENSE](LICENSE) file for details.
