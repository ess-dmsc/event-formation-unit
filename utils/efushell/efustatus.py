#!/usr/bin/python

from EFUMetrics import Metrics
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-i", metavar='ipaddr', help = "server ip address (default 127.0.0.1)",
                    type = str, default = "127.0.0.1")
parser.add_argument("-p", metavar='port', help = "server tcp port (default 8888)",
                    type = int, default = 8888)
args = parser.parse_args()

print("")
print("EFU Stats:")
print("Connection info. ip address: %s, tcp port: %s" % (args.i, args.p))
metrics = Metrics(args.i, args.p)

print(metrics._get_efu_command("VERSION_GET"))
print(metrics._get_efu_command("DETECTOR_INFO_GET"))

res = metrics._get_efu_command("CMD_GET_COUNT")
numcmds = int(res.split()[1])
print("Available commands: (CMD_GET 1 - %s)" % (numcmds))
for cmdnum in range(1, numcmds + 1):
    print(metrics._get_efu_command("CMD_GET " + str(cmdnum)).split()[1])

res = metrics._get_efu_command("STAT_GET_COUNT")
numstats = int(res.split()[1])
print("Available stats: (STAT_GET 1 - %s)" % (numstats))
for statnum in range(1, numstats + 1):
    print(metrics._get_efu_command("STAT_GET " + str(statnum)).split()[1])
