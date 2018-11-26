#!/usr/bin/python

from EFUMetrics import Metrics
import argparse, sys

svr_ip_addr = "127.0.0.1"
svr_tcp_port = 8888

parser = argparse.ArgumentParser()
parser.add_argument("-i", metavar='ipaddr', help = "server ip address (default 127.0.0.1)", type = str)
parser.add_argument("-p", metavar='port', help = "server tcp port (default 8888)", type = int)
args = parser.parse_args()

if args.i is not None:
    svr_ip_addr = args.i

if args.p is not None:
    svr_tcp_port = args.p

try:
  metrics = Metrics(svr_ip_addr, svr_tcp_port)

  res = metrics._get_efu_command("STAT_GET 1")
  packets = int(res.split()[2])
  print("efu is running")
except:
  print("efu is not running")
  sys.exit(1)

sys.exit(0)
