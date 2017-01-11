## Prerequisites
The following libraries and utilities must be installed for full functionality.
However all are optional. 

* Zookeeper
* Apache Kafka
* librdkafka - needed for output_thread (can be disabled by NOKAFKA=y)
* Googletest - needed for 'make test, make runtest' targets
* lcov - needed for 'make coverage' target
* valgrind - needed for 'make valgrind' target

See details about how to start Kafka (if running locally) and Zookeeper
further down.

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
V | verbose
COV | instrument for test coverage
PROF | instrument for profiling (not useful for shared libraries)
NOKAFKA | removes Kafka dependencies
GTEST=path_to_gtest | specify an alternative google test library


Except for GTEST, options are disabled by omission and  enabled by
assignment. ex: make COV=y


__Run in terminal window__

In principle as simple as

`> ./efu2`

However for practical purposes

`> export LD_LIBRARY_PATH=/usr/lib64/ ; ./efu2`

The efu process creates a number of pthreads, currently on cpus 12, 13, 14.


__Run in another terminal window__

`> taskset -c 16 ./cspecgen`

Both programs shows help with -h option.


If your workstation has less than 16 hyperthreads you need to modify
the code. This will probably be changed soon.



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

