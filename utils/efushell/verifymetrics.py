#!/usr/bin/python

from EFUMetrics import Metrics
import argparse, sys


def verify_metric(ip_address, port, validation):
    metrics = Metrics(ip_address, port)
    metrics.get_all_metrics(metrics.get_number_of_stats())

    m = validation.split(":")
    if len(m) != 2:
        print("Illegal metric specification: {} (hint use name:value)".format(validation))
        sys.exit(1)
    name = m[0]
    value = int(m[1])
    f = lambda x, y: (x == y, "==")
    if (m[1])[0] == "+":
        f = lambda x, y: (x >= y, ">=")
        value = int(m[1][1:])
    retval = metrics.return_metric(name)
    res, op = f(retval, value)

    return res, name, op, value, retval


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", metavar='ipaddr', help="server ip address (default 127.0.0.1)",
                        type=str, default="127.0.0.1")
    parser.add_argument("-p", metavar='port', help="server tcp port (default 8888)",
                        type=int, default=8888)
    parser.add_argument('metrics', metavar='metrics', nargs='+',
                        help='grafana metrics to be verified, examples: efu.rx_packets:1234 - strict match. efu.rx_packets:+1234 at least)')
    args = parser.parse_args()

    for test_metric in args.metrics:
        res, name, op, value, retval = verify_metric(args.i, args.p, test_metric)
        if not res:
            print("Validation failed for %s: expected %s %d, got %d" % (name, op, value, retval))
            sys.exit(1)
    print("OK")
