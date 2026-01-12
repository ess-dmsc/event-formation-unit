#!/usr/bin/env python3

import argparse
import subprocess


def setCalibrationMode(args):
    """
    Set or query the calibration mode for a running EFU.

    If the option '-m' or '--mode' is used, the requested calibration mode is set. Otherwise, the
    current calibration mode is queried and printed.
    """
    cmd = ''

    # Set up commandline 
    if args.mode is not None:
        mode = 1 if args.mode == 'on' else 0
        cmd = f'echo CALIB_MODE_SET {mode} | nc -N {args.i} {args.p}'
    else:
        cmd = f'echo CALIB_MODE_GET | nc -N {args.i} {args.p}'

    # Run command
    p = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    response = p.stdout.readlines()
    if not response:
        print("Failed to query calibration mode")
        return False
    response = response[0].decode("utf-8")

    # Setting
    if args.mode is not None:
        if response == '<OK>':
            print ('Calibration mode successfully set to "{mode}"'.format(mode=args.mode))
        else:
            print ('Failed to set calibration mode')

    # Getting
    else:
        mode = 'off' if '0' in response else 'on'
        print ('Calibration mode is "{mode}"'.format(mode=mode))

    return True

if __name__ == "__main__":
    # Extract args
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", metavar='ipaddr', help = "server ip address (default 127.0.0.1)",
                        type=str, default="127.0.0.1")

    parser.add_argument("-p", metavar='port', help="server tcp port (default 8888)",
                        type=int, default=8888)

    parser.add_argument("-m", "--mode", metavar='mode', help="Set the calibration mode (valid values are 'on' or 'off')",
                        type=str, choices=['on', 'off'])
    
    args = parser.parse_args()

    # Perform requested actions
    setCalibrationMode(args)
