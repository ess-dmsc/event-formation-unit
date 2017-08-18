## Prerequisites
The following libraries and utilities must be installed for full functionality.
However most are optional.

* Zookeeper
* Apache Kafka
* librdkafka - needed for output_thread (can be disabled by NOKAFKA=y)
* Googletest - needed for 'make test, make runtest' targets
* HDF5
* lcov/gcov/gcovr - needed for 'make coverage' target
* valgrind - needed for 'make valgrind' target


## Build and Run

To build:

    > cd event-formation-unit
    > mkdir build
    > cd build
    > cmake [options] ..
    > make [target]


__Makefile targets and options__

Target                | Description
-------------         | -------------
hints                 | DEFAULT and NORMATIVE target and option guide
help                  | lists all available make targets
runtest               | run test executables, generate xml reports
coverage              | generate test coverage report in html/index.html
valgrind              | run valgrind on tests, generate reports

Option         | Description
-------------  | -------------
NOKAFKA        | removes Kafka dependencies


### Execution

__Run in terminal window__

`> ./efu2 -d detector`


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
