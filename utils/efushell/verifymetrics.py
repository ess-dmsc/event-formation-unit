#!/usr/bin/python

from EFUMetrics import Metrics
import argparse, sys

svr_ip_addr = "127.0.0.1"
svr_tcp_port = 8888


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", metavar='ipaddr', help = "server ip address (default 127.0.0.1)", type = str)
    parser.add_argument("-p", metavar='port', help = "server tcp port (default 8888)", type = int)
    parser.add_argument('metrics', metavar='M', nargs='+',
                        help='metrics to be verified')

    args = parser.parse_args()
    if args.i is not None:
        svr_ip_addr = args.i

    if args.p is not None:
        svr_tcp_port = args.p

    metrics = Metrics(svr_ip_addr, svr_tcp_port)
    metrics.get_all_metrics(metrics.get_number_of_stats())

    for validation in args.metrics:
        m = validation.split(":")
        if len(m) != 2:
            print("Illegal metric specification: %s (hint use name:value)" % (validation))
            sys.exit(1)
        name = m[0]
        value = int(m[1])
        if not metrics.compare_metric(name, value):
            print("Validation failed for %s" % (name))
            sys.exit(1)
    print("OK")
