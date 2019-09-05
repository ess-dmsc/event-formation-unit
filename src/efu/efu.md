### Starting the EFU

To start the EFU with the __sonde__ pipeline from the __build__ directory:

`> ./bin/efu -d lib/sonde --nohwcheck`


There are quite a few additional command line options, but these ones are
the most used

Cmd option          | Description
-------------       | -------------
-p                  | UDP port for detector data
-g                  | IP address for Graphite (metrics) server
-b                  | Kafka broker "ipaddress:port"
-f                  | Configurations file (detector specific)

To get further help

`> ./efu -h`
