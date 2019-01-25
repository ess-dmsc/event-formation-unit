#!/usr/bin/python

from EFUMetrics import Metrics
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-i", metavar='ipaddr', help = "server ip address (default 127.0.0.1)",
                    type = str, default = "127.0.0.1")
parser.add_argument("-p", metavar='port', help = "server tcp port (default 8888)",
                    type = int, default = 8888)
parser.add_argument("-v", help = "Dump stats in format suitable for verifymetrics.py", action = 'store_true')

args = parser.parse_args()

print("")
metrics = Metrics(args.i, args.p)

res = metrics._get_efu_command("STAT_GET_COUNT")
numstats = int(res.split()[1])
print("Available stats:")
verify = ""
for statnum in range(1, numstats + 1):
    res = metrics._get_efu_command("STAT_GET " + str(statnum))
    verify = verify + res.split()[1] + ":" + res.split()[2] + " "
    print(res)

if args.v:
    print
    print verify
