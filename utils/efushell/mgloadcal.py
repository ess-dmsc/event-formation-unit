#!/usr/bin/python

from SocketDriver import SimpleSocket
import argparse

svr_ip_addr = "127.0.0.1"
svr_tcp_port = 8888

parser = argparse.ArgumentParser()
parser.add_argument("-i", metavar='ipaddr', help = "server ip address (default 127.0.0.1)", type = str)
parser.add_argument("-p", metavar='port', help = "server tcp port (default 8888)", type = int)
parser.add_argument('calibfile', help='multigrid calibration file for loading')
args = parser.parse_args()

if args.i is not None:
    svr_ip_addr = args.i

if args.p is not None:
    svr_tcp_port = args.p

print("Connection info. ip address: %s, tcp port: %s" % (svr_ip_addr, svr_tcp_port))
driver = SimpleSocket(svr_ip_addr, svr_tcp_port)

calibfile=args.calibfile
print("attempting to load file {}".format(calibfile))

res = driver.Ask('CSPEC_LOAD_CALIB ' + calibfile)
print res
