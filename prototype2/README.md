## Prerequisites
The following libraries and utilities must be installed for full functionality.
However all are optional.

* Zookeeper
* Apache Kafka
* librdkafka - needed for output_thread (can be disabled by NOKAFKA=y)
* Googletest - needed for 'make test, make runtest' targets
* HDF5
* lcov - needed for 'make coverage' target
* valgrind - needed for 'make valgrind' target


## Build and Run

To build:

`> make`

__Makefile targets and options__

Target            | Description
-------------         | -------------
all | default target, builds executables and loadable libraries
test | build test executables
runtest | run test executables, generate xml reports
coverage | generate test coverage report in html/index.html
valgrind | run valgrind on tests, generate reports
clean | clean prototype2/
realclean |  also clean libs/
release | checks for missing copyright notice and doxygen `@file` comments
doxygen | generate doxygen documentation


Option         | Description
-------------  | -------------
V              | verbose
COV            | instrument for test coverage
PROF           | instrument for profiling (not useful for shared libraries)
NOKAFKA        | removes Kafka dependencies
KAFKAINC       | directory for Kafka header files
KAFKALIB       | directory for Kafka libraries
HDF5           | build with support for reading HDF5 files
HDF5INC        | directory for hdf5 header files
HDF5LIB        | directory for hdf5 libraries
GTEST=path     | specify an alternative location of google test library (def unset)

Except for GTEST, options are disabled by omission and  enabled by
assignment. ex: make COV=y

### Platform differences
On CentOS build as follows
    > make HDF5=y

On (Mortens) Ubuntu use the following arguments to build

    > make NOKAFKA=y HDF5=y HDF5INC=/usr/include/hdf5/serial HDF5LIB=/usr/lib/x86_64-linux-gnu/hdf5/serial
    > make KAFKAINC=/usr/local/include KAFKALIB=/usr/lib64

On (Mortens) Mac
    > make  NOKAFKA=y HDF5=y HDF5INC=/usr/local/include HDF5LIB=/usr/local/lib
    > make NOKAFKA=y

### Execution

__Run in terminal window__

`> export LD_LIBRARY_PATH=/usr/lib64/ ; ./efu2`

library path points to librdkafka. The efu process creates a number of pthreads, currently on cpus 12, 13, 14.


__Run in another terminal window__

`> taskset -c 16 ./cspecgen`

Both programs shows help with -h option.


##Zookeeper
Starting zookeeper is only necessary once

`> ./bin/zookeeper-server-start.sh config/zookeeper.properties`


##Kafka broker
Starting Kafka broker is only necessary once

`> ./bin/kafka-server-start.sh config/server.properties`


Both commands are relative to the kafka installation directory.

You can also use the bash script kafkaservice from utils/

`> ./kafkaservice start`

`> ./kafkaservice stop`
