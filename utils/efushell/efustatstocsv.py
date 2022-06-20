#!/usr/bin/python

from EFUMetrics import Metrics
import argparse
import csv

parser = argparse.ArgumentParser()
parser.add_argument("-i", metavar='ipaddr', help = "server ip address (default 127.0.0.1)",
                    type = str, default = "127.0.0.1")
parser.add_argument("-p", metavar='port', help = "server tcp port (default 8888)",
                    type = int, default = 8888)
parser.add_argument("-v", help = "Dump stats in format suitable for verifymetrics.py", action = 'store_true')
parser.add_argument("-f", type=str, default="efustats.csv", help="Filename to save csv file under")

args = parser.parse_args()

print("")
metrics = Metrics(args.i, args.p)

res = metrics._get_efu_command("STAT_GET_COUNT")
numstats = int(res.split()[1])
print("Available stats ({}):".format(numstats))
verify = ""
stats_dict = {}
for statnum in range(1, numstats + 1):
    res = metrics._get_efu_command("STAT_GET " + str(statnum))
    verify = verify + str(res.split()[1]) + ":" + str(res.split()[2]) + " "
    stats_dict[res.split()[1].decode('utf-8')] = res.split()[2].decode('utf-8')

print(stats_dict)
with open(args.f, 'w') as csvfile: 
    for key in stats_dict.keys():
        csvfile.write("%s, %s\n" % (key, stats_dict[key]))

if args.v:
    print(verify)

