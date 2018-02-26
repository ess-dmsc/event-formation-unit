### Starting the EFU

To start the EFU with the __mgcncs2__ pipeline from the __build__ directory:

`> ./bin/efu -d ./lib/mgcncs2`

If the efu and detector plugins (.so files) are located in the __same__ directory:

`> ./efu -d mgcncs2`

There are quite a few additional command line options, but these ones are
the most used

Cmd option          | Description
-------------       | -------------
-p                  | UDP port for detector data
-g                  | IP address for Graphite (metrics) server
-b                  | Kafka broker "ipaddress:port"
-c                  | CPU core id for the first processing thread (-5 == ignore)

To get further help

`> ./efu -h`
