# Event Formation Unit
[![License (2-Clause BSD)](https://img.shields.io/badge/license-BSD%202--Clause-blue.svg)](https://github.com/ess-dmsc/event-formation-unit/blob/master/LICENSE) [![DOI](https://zenodo.org/badge/80731668.svg)](https://zenodo.org/badge/latestdoi/80731668)

| Branch | Pipeline Status | Coverage |
|--------|----------------|----------|
| master | [![pipeline status](https://gitlab.esss.lu.se/ecdc/ess-dmsc/event-formation-unit/badges/master/pipeline.svg)](https://gitlab.esss.lu.se/ecdc/ess-dmsc/event-formation-unit/-/commits/master) | [![coverage report](https://gitlab.esss.lu.se/ecdc/ess-dmsc/event-formation-unit/badges/master/coverage.svg)](https://gitlab.esss.lu.se/ecdc/ess-dmsc/event-formation-unit/-/commits/master) |

> **📊 Detailed Coverage Reports:** Interactive HTML coverage reports are available at [GitLab Pages](http://pages.ess.eu/ess-dmsc/event-formation-unit/coverage.html). These reports provide line-by-line coverage analysis and are automatically updated with each merge to master.

This project implements processing of neutron detector event data into neutron events. Pipelines
for processing of raw data from Gd-GEM, Muli-Grid, Multi-Blade, SoNDe as well as a few other detectors
have been implemented. Mostly implemented in C/C++.

## Documentation
For more details on the file structure, architecture, primitives see [documentation/README.md](documentation/README.md)

## Changelog Generator
This project includes an automated changelog generator that creates formatted release notes from git commits. The tool uses [uv](https://github.com/astral-sh/uv) for dependency management and requires no manual setup. See [utils/changelog/README.md](utils/changelog/README.md) for details.

Quick usage from project root:
```bash
# Generate changelog since last tag (recommended)
./generate-changelog.sh --from-commit v1.0.0

# Or run directly with uv
uv run utils/changelog/generate_changelog.py --from-commit abc1234 --times

# Script can also be run directly (with executable permissions)
utils/changelog/generate_changelog.py --from-commit abc1234
```

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

## Install and Configure Conan

#### 1. Create and activate a virtual environment

```bash
python3 -m venv venv
source venv/bin/activate
```
#### 2. Install Conan (version < 2)
```bash
pip install "conan<2"
```

#### 3. Verify Conan installation

```bash
conan --version
```

#### 4. Install the Conan configuration
This includes supported Conan profiles and access to the ECDC internal package repository:

```bash
conan config install http://github.com/ess-dmsc/conan-configuration.git
```

## Release Build

#### 1. Create a build directory:
```bash
mkdir build && cd build
```

#### 2. Install Dependencies with Conan 

The ECDC Conan configuration includes a predefined profile: `linux_x86_64_gcc11`. 
This profile is well-tested and has many prebuilt binaries available in our remote repository. 
It's also the one we use in automated ci builds for this project.

If you're working on a different architecture (e.g. Apple Silicon) or using a different compiler (e.g. GCC 13), note that no predefined profiles exist in our config for those setups. 
In such cases, you'll need to use Conan's locally generated `default` profile. 


Verify your default profile exists:
```bash
conan profile show default
```

To install dependencies using your local profile:
```bash
conan install .. --build=missing
```

To use the ECDC-supported Linux x86 GCC 11 profile instead, add:
```bash
--profile=linux_x86_64_gcc11
```

#### 3. Configure the project with CMake:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

#### 4. Build the project:
```bash
make -j$(nproc)
```

## Debug build for unit tests and coverage

#### 1. Create a build directory:
```bash
mkdir build && cd build
```

#### 2. Install dependencies with Conan:
```bash
conan install .. --build=missing -s build_type=Debug
```

#### 3. Configure the project with CMake:
```bash
cmake -DCOV=ON -DCMAKE_BUILD_TYPE=Debug -DGOOGLE_BENCHMARK=ON ..
```

#### 4. Run unit tests:

```bash
make runtest
```

#### 5. Generate coverage report:
```bash
make coverage_all
```

View the report:
```bash
open coverage/coverage.html
```

#### 6. To run a memory leak test, run:
```bash
make valgrind
```

## Building Doxygen docmentation

[Doxygen](https://www.doxygen.nl/) documentation for the EFU C++ classes can be build by running

```bash
make doxygen
```

After the documentation has been build, the doxygen documentation tree can be accesed in the cmake build directory, by opening the file `doxygen/html/index.html`. 


## Running the event formation application

An example of the commands required to run an event formation pipeline (in this case the *freia* pipeline) follows:

```bash
make efu freia
cd bin
./efu -d ../modules/freia --nohwcheck
```

Note you will need to provide a config file in the case of the *freia* module as well.

To get the available command line arguments, use `-h` or `--help`. This works when providing a detector module argument as well. For example:
```bash
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
* Tibor Bukovics
* Michael Christiansen

See also the [list of contributors](https://github.com/ess-dmsc/event-formation-unit/graphs/contributors) on Github.

## License

This project is licensed under the BSD-2 License see the [LICENSE](LICENSE) file for details.
