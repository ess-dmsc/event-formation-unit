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
    extra_quantities = []
    if args.q:
        extra_quantities = [q.strip() for q in args.q.split(',')]
    quantities = [
        'receive.packets',
        'receive.bytes',

        'readouts.count',
        'readout.count',

        'events.count',
        'events.ibm',
    ] + extra_quantities

    # Store stat keys here
    keys = []

    # Find indices and keys for the specified props
    indices = []
    for index in range(1, statCount + 1):
        stat = str(metrics._get_efu_command("STAT_GET " + str(index)), 'utf-8')
        stat = stat.split()[1].strip()
        if any([stat.endswith(quantity) for quantity in quantities]):
            indices.append(index)
            keys.append(stat)

    # Length of longest key
    length = max(len(key) for key in keys) + 1

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
            rates = []
            for index in range(len(indices)):
                delta = float(t1 - t0)
                rate = (r1[index] - r0[index])/delta
                rate = '{:.1f}'.format(rate)
                rates.append((keys[index], rate))

            r_length = max(len(r) for (key, r) in rates)
            for (key, rate) in rates:
                print("{} {}".format(key.ljust(length), rate.rjust(r_length)))

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

    parser.add_argument("-q", metavar='quantities', help = "Print rates for a comma separated list of quantities",
                        type = str, default = "")

    args = parser.parse_args()

    # Start reading rates
    getRates(args)
