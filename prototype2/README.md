## Prerequisites
The following libraries and utilities must be installed for full functionality.
However most are optional.

* librdkafka - needed for output_thread (can be disabled by NOKAFKA=y)
* Googletest - needed for 'make test, make runtest' targets
* HDF5 / H5CC
* lcov/gcov/gcovr - needed for 'make coverage' target
* valgrind - needed for 'make valgrind' target


## Building the EFU

To build:

    > cd event-formation-unit
    > mkdir build
    > cd build
    > cmake [options] ..
    > make [target]


__Makefile targets and options__

See options and targets by

    > make hints

Two important options, however, are

Option                | Description
-------------         | -------------
-DDUMPTOFILE          | Activates code for writing readout to disk (default OFF)
-DUSE_OLD_ABI         | On CentOS we currently use the old ABI which can conflict on <br> newer distributions (default OFF)


Target                | Description
-------------         | -------------
hints                 | DEFAULT and NORMATIVE target and option guide
help                  | lists all available make targets
runtest               | run test executables, generate xml reports
coverage              | generate test coverage report in html/index.html
valgrind              | run valgrind on tests, generate reports


### Starting the EFU

To start the EFU with the __mgcncs2__ pipeline from the __build__ directory:

`> ./prototype2/efu2 -d prototype2/multigrid/mgcncs2`

If the efu and detector plugins (.so files) are located in the __same__ directory:

`> ./efu2 -d mgcncs2`

There are quite a few additional command line options, but these ones are
the most used

Cmd option          | Description
-------------       | -------------
-p                  | UDP port for detector data
-g                  | IP address for Graphite (metrics) server
-b                  | Kafka broker "ipaddress:port"
-c                  | CPU core id for the first processing thread (-5 == ignore)

To get further help

`> ./efu2 -h`
