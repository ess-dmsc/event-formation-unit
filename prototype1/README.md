

To run this prototype:

`> make`


Run in terminal window A:

`> taskset -c 15 ./efu`

Run in terminal window B:

`> taskset -c 16 ./bulkdatagen`

The efu creates a number of pthreads, currently on cpus 12, 13, 14.

If your workstation has less than 16 hyperthreads you need to modify
the code. This will probably be changed soon.
