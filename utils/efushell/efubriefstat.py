#!/usr/bin/env python3

from EFUMetrics import Metrics
import argparse

def uptime_to_string(time):
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
args = parser.parse_args()


metrics = Metrics(args.i, args.p)

cmd, long, date, time, user, version, hash = metrics._get_efu_command("VERSION_GET").decode('utf-8').split()
cmd, detector = metrics._get_efu_command("DETECTOR_INFO_GET").decode('utf-8').split()
cmd, name, uptime = metrics._get_efu_command("STAT_GET_NAME main.uptime").decode('utf-8').split()

uptime = uptime_to_string(int(uptime))

print(f'Detector  {detector}')
print(f'Build     {date} {time}')
print(f'Version   {hash} {version}')
print(f'uptime    {uptime}')
