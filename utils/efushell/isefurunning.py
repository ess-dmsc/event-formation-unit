#!/usr/bin/python

from EFUMetrics import Metrics
import argparse, sys

parser = argparse.ArgumentParser()
parser.add_argument("-i", metavar='ipaddr', help = "server ip address (default 127.0.0.1)",
                    type = str, default = "127.0.0.1")
parser.add_argument("-p", metavar='port', help = "server tcp port (default 8888)",
                    type = int, default = 8888)
args = parser.parse_args()

try:
  metrics = Metrics(args.i, args.p)

  res = metrics._get_efu_command("STAT_GET 1")
  packets = int(res.split()[2])
  print("efu is running")
except:
  print("efu is not running")
  sys.exit(1)

sys.exit(0)
