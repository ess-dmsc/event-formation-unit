## Prerequisites
The following libraries and utilities must be installed

* Zookeeper
* Apache Kafka
* librdkafka
* Googletest

See details about how to start Kafka (if running locally) and Zookeeper at the end.

## Build and Run

To build:

`> make`



__Run in terminal window__

In principle as simple as 

`> ./efu`

However for practical purposes 

`> export LD_LIBRARY_PATH=/usr/local/lib`


`> taskset -c 15 ./efu`

Where the first line is done once in the lifetime of a terminal session.

The efu process creates a number of pthreads, currently on cpus 12, 13, 14.


__Run in another terminal window__

`> taskset -c 16 ./bulkdatagen`

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
