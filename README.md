# Event Formation Unit
[![License (2-Clause BSD)](https://img.shields.io/badge/license-BSD%202--Clause-blue.svg)](https://github.com/ess-dmsc/event-formation-unit/blob/master/LICENSE) [![DOI](https://zenodo.org/badge/80731668.svg)](https://zenodo.org/badge/latestdoi/80731668) [![Build Status](https://jenkins.esss.dk/dm/job/ess-dmsc/job/event-formation-unit/job/master/badge/icon)](https://jenkins.esss.dk/dm/job/ess-dmsc/job/event-formation-unit/job/master/)

This project implements processing of neutron detector event data into neutron events. Pipelines
for processing of raw data from Gd-GEM, Muli-Grid, Multi-Blade, SoNDe as well as a few other detectors
have been implemented. Mostly implemented in C/C++.

## Documentation
For more details on the file structure, architecture, primitives see [documentation/README.md](documentation/README.md)

## Getting started

The [essdaq repository](https://github.com/ess-dmsc/essdaq) has scripts for automatically
downloading and compiling this project. Instructions for manually compiling the event
formation unit software follow.

### Prerequisites

To build and run this software the following dependencies are required.

* [**Conan**](https://conan.io) The conan script has to be available in the current ``$PATH``. Use the latest version smaller than 2.
* [**CMake**](https://cmake.org) At least CMake version 2.8.12 is required. Some packages that requires more recent versions of CMake will download this as a dependency.
* [**bash**](https://www.gnu.org/software/bash/) For properly setting paths to the conan provided dependencies.
* A recent C/C++ compiler with support for C++14.

Note also that for additional functionality, you might want to install the following dependencies manually:

* [**libpcap**](http://www.tcpdump.org) —Network monitoring
* [**Valgrind**](http://valgrind.org) — Memory usage (and other) tests
* [**gcovr**](https://gcovr.com/en/stable/index.html), [**lcov**](https://lcov.readthedocs.io/en/latest/#lcov),  and [**gcov**](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html) — Generate coverage reports

### Conan

Conan is a Python package used to download dependencies. To install the latest version smaller than 2, run 

```
pip install "conan<2"
```

and ensure that the directory conatining the conan executable has been added to your path.  

For conan to know where the dependencies can be downloaded from, the ECDC package repository must be added by running the following command

```conan remote add ecdc-conan-release https://artifactory.esss.lu.se/artifactory/api/conan/ecdc-conan-release```


### Building

Run the following commands:

```
git clone https://github.com/ess-dmsc/event-formation-unit.git

cd event-formation-unit

mkdir build

cd build

cmake ..

make
```

Note, by utilizing several processors, you can speed up the build process by running parallel compiler jobs. You do this by passing the `-j` option to make. For example, to launch eight parallel jobs run make as

```
make -j 8
```

The total number of available processors on can be queried by calling the `nproc` command. To use all available processors, run make like this

```
make -j $(nproc)
```

#### Building with Ubuntu verions 16 or larger 
Wen using conan to provide the dependencies, an extra option has to be provided:
`--settings compiler.libcxx=libstdc++11`. Thus the call to conan turns into:

```
conan install --build=outdated .. --settings compiler.libcxx=libstdc++11
```

## Running the tests

### Unit tests
To run the unit tests for this project, run the following commands:

```
make runtest
```

### Other tests

It is also possible to get a test coverage report if the required prerequisites have been installed. 
For this you have to enable coverage test in the makefile with cmake. 
To enable coverage test for makefiles, run

```
cmake -DCOV=Y
```
Then to get the coverage report, run

```
make coverage
```

To run a memory leak test (using Valgrind), run:

```
make valgrind
```

## Building Doxygen docmentation

[Doxygen](https://www.doxygen.nl/) documentation for the EFU C++ classes can be build by running

```
make doxygen
```

After the documentation has been build, the doxygen documentation tree can be accesed in the cmake build directory, by opening the file `doxygen/html/index.html`. 


## Running the event formation application

An example of the commands required to run an event formation pipeline (in this case the *freia* pipeline) follows:

```
make efu freia
cd bin
./efu -d ../modules/freia --nohwcheck
```

Note you will need to provide a config file in the case of the *freia* module as well.

To get the available command line arguments, use `-h` or `--help`. This works when providing a detector module argument as well. For example:
```
./efu -d ../modules/freia -h
```

## Contributing

Please read the [CONTRIBUTING.md](CONTRIBUTING.md) file for details on our code of
conduct and the process for submitting pull requests to us.

## Authors

* Morten Jagd Christensen
* Martin Shetty
* Jonas Nilsson
* Jenny Walker

See also the [list of contributors](https://github.com/ess-dmsc/event-formation-unit/graphs/contributors) on Github.

## License

This project is licensed under the BSD-2 License see the [LICENSE](LICENSE) file for details.
