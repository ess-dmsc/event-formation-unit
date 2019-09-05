# Running artefacts

If you have downloaded the binary artifacts of this repository compiled using our Jenkins build nodes, you will need to manually set the path to the *lib* directory. This is done by setting the `LD_LIBRARY_PATH` environment variable. E.g. to run the EFU using the sonde module, execute the following line:

```
LD_LIBRARY_PATH=../lib ./efu -d ../lib/sonde
```
