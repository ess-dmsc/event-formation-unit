#!/usr/bin/env python3

from EFUMetrics import Metrics
import argparse
import time

def getRates(args):
    """
    Calculate the rate of change for a pre-selected set of EFU statistical
    quantities.
    """
    # Initialize EFU metrics
    metrics = Metrics(args.i, args.p)
    stat = metrics._get_efu_command("STAT_GET_COUNT")
    statCount = int(stat.split()[1])

    # Properties we should get rates for
    props = [
        'receive.packets',
        'receive.bytes',
        'readouts.count',
        'events.count',
    ]

    # Store stat keys here
    keys = []

    # Find indices and keys for the specified props
    indices = []
    for index in range(1, statCount + 1):
        stat = str(metrics._get_efu_command("STAT_GET " + str(index)), 'utf-8')
        if any([p in stat for p in props]):
            indices.append(index)
            keys.append(stat.strip().split()[1])

    # Length of longest key
    length = max(len(key) for key in keys)

    # Initialize variables
    dt = args.d
    t0 = None
    t1 = None

    r0 = None
    r1 = None

    caption = "Change per second"
    caption = caption + '\n' + '-'*(len(caption) + 1)

    while (True):
        # Get stats and store time
        r1 = []
        t1 = time.time()
        for index in indices:
            stat = str(metrics._get_efu_command("STAT_GET " + str(index)), 'utf-8')
            stat = stat.strip().split()
            r1.append(int(stat[2]))

        # Print new rates
        if ((t0 is not None) and (r0 is not None)):
            print ("\n{}".format(caption))
            for index in range(4):
                delta = float(t1 - t0)
                print("{}: {:.1f}".format(keys[index].ljust(length), (r1[index] - r0[index])/delta))

        # Store old time and readout
        t0 = t1
        r0 = r1

        # Take a break before next readout
        time.sleep(dt)

if __name__ == "__main__":
    # Extract args
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", metavar='ipaddr', help = "server ip address (default 127.0.0.1)",
                        type = str, default = "127.0.0.1")

    parser.add_argument("-p", metavar='port', help = "server tcp port (default 8888)",
                        type = int, default = 8888)

    parser.add_argument("-d", metavar='delta', help = "Sample time between each stat readout",
                        type = float, default = 1.0)

    args = parser.parse_args()

    # Start reading rates
    getRates(args)
