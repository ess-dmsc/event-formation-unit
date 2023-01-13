#!/usr/bin/env python3

from EFUMetrics import Metrics
import argparse


def uptime_to_string(time):
    time = int(time)
    day = time // (24 * 3600)
    time = time % (24 * 3600)
    hour = time // 3600
    time %= 3600
    minutes = time // 60
    time %= 60
    seconds = time
    return f'{day} days {hour} hrs, {minutes} min, {seconds} s'


parser = argparse.ArgumentParser()
parser.add_argument("-i", metavar='ipaddr', help = "server ip address (default 127.0.0.1)",
                    type = str, default = "127.0.0.1")
parser.add_argument("-p", metavar='port', help = "server tcp port (default 8888)",
                    type = int, default = 8888)
parser.add_argument("--version", help = "return version and commit hash",
                    action='store_true')
parser.add_argument("--name", help = "return detector name", action='store_true')
parser.add_argument("--date", help = "return build date", action='store_true')
parser.add_argument("--uptime", help = "return uptime", action='store_true')

args = parser.parse_args()


metrics = Metrics(args.i, args.p)

cmd, long, date, time, user, version, hash = metrics._get_efu_command("VERSION_GET").decode('utf-8').split()
cmd, detector = metrics._get_efu_command("DETECTOR_INFO_GET").decode('utf-8').split()
cmd, name, uptime = metrics._get_efu_command("STAT_GET_NAME main.uptime").decode('utf-8').split()
uptime = uptime_to_string(uptime)

if args.name:
    print(f'{detector}')
elif args.version:
    print(f'{hash} {version}')
elif args.date:
    print(f'{date} {time}')
elif args.uptime:
    print(f'{uptime}')
else:
    print(f'detector  {detector}')
    print(f'buildtime {date} {time}')
    print(f'version   {hash} {version}')
    print(f'uptime    {uptime}')
