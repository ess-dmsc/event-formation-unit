#!/usr/bin/env python3

from EFUMetrics import Metrics
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-i", metavar='ipaddr', help = "server ip address (default 127.0.0.1)",
                    type = str, default = "127.0.0.1")
parser.add_argument("-p", metavar='port', help = "server tcp port (default 8888)",
                    type = int, default = 8888)
parser.add_argument("-z", help = "don't show counters with value 0 (zero suppression)", action = 'store_true')
parser.add_argument("-v", help = "dump stats in format suitable for verifymetrics.py", action = 'store_true')
parser.add_argument("-d", help = "add 3-digit separators to counters", action = 'store_true')

args = parser.parse_args()

print("")
metrics = Metrics(args.i, args.p)

result = metrics._get_efu_command("STAT_GET_COUNT")
numstats = int(result.split()[1])
print("Available stats ({}):".format(numstats))
verify = ""

results = []
for statnum in range(1, numstats + 1):
    result = metrics._get_efu_command("STAT_GET " + str(statnum))
    verify = verify + str(result.split()[1]) + ":" + str(result.split()[2]) + " "
    result=str(result,'utf-8')
    if args.z and result.endswith(" 0"):
        continue
    results.append(result)

# Find the string length of the largest quantity and count
q_length = max([len(result.split()[1].strip()) for result in results]) + 1
c_length = max([len(result.split()[2].strip()) for result in results]) + 1

# Check if we use 3-digit separators
if args.d:
    c_length += c_length//3

# Print quantity and count in two aligned columns
for result in results:
    (key, count) = [r for r in result.split()][1:]
    if args.d:
        count = '{:,}'.format(int(count))
    print("{} {}".format(key.ljust(q_length), count.rjust(c_length)))

if args.v:
    print(verify)
