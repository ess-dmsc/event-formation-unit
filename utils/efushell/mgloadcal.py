#!/usr/bin/python

import sys
from SocketDriver import SimpleSocket

driver = SimpleSocket("localhost", 8888)


calibfile=sys.argv[1]
print("attempting to load file %s" % (calibfile))

res = driver.Ask('CSPEC_LOAD_CALIB ' + calibfile)
print res
