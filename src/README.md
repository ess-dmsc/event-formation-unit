
## Directories

Source files for efu, detector modules and data generators.

Directory             | Function
-------------         | -------------
common                | common detector code
efu                   | EFU main application code
generators            | detector data generators (for test)
modules               | detector implementations

## external modules
It is possible to build modules with sources outside the efu repository. To
do this we make use of EFU_EXTERNAL_DIR which is a cmake variable containing
a list of directories with cmake modules to be included.

    > cmake -D EFU_EXTERNAL_DIR= "/tmp/thisrepo;/tmp/thatrepo" ..
