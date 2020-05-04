
# Qt based efu launcher
This tool helps build a command line for launching efu.

## qtefu.py
The user speficies three directories: basedir, efudir and datadir. The latter two must
be relative to basedir.

The prgram then finds the relevant binary files (efu executable and detector libraries),
and configuration and calibration files.

The user then selects the relevant files form the drop down menus and presses OK
to launch the EFU.

## qtdatagen.py
Still under development

## Custom configuration
To customise the basedir and the location of efu binaries and configuration files, you
can edit the efucfg file, rename it to .efucfg and place it in your homedir.

## Prerequisites
We need Qt5 support which can be gotten by
    pip3 install PyQt5
