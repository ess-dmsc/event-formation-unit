#!/usr/bin/python

from EFUMetrics import Metrics
import argparse, sys

parser = argparse.ArgumentParser()
parser.add_argument('stat', metavar='stat', type=str, help="stat name (sub)string");
parser.add_argument("-i", metavar='ipaddr', help = "server ip address (default 127.0.0.1)", type = str, default = "127.0.0.1")
parser.add_argument("-p", metavar='port', help = "server tcp port (default 8888)", type = int, default = 8888)
args = parser.parse_args()

my_metric = args.stat

metrics = Metrics(args.i, args.p)

res = metrics._get_efu_command("STAT_GET_COUNT")
numstats = int(res.split()[1])
for statnum in range(1, numstats + 1):
    stat = metrics._get_efu_command("STAT_GET " + str(statnum))
    if my_metric in stat:
        print(stat.split(" ")[2])
