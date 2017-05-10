#!/usr/bin/env python
# -*- coding: utf-8 -*-

import re
import time

proc_file = open("/proc/stat", "r")

def getStats():
    proc_file.seek(0)
    in_data = proc_file.read()
    ret_dict = {}
    lines = in_data.split("\n")
    line_re = "(cpu\d{1,2})\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)"
    for line in lines:
        re_res = re.search(line_re, line)
        if (re_res != None):
            temp_dict = {}
            names = ["user", "nice", "system", "idle", "iowait", "irq", "softirq"]
            for i, name in enumerate(names):
                temp_dict[name] = int(re_res.group(i + 2))
            ret_dict[re_res.group(1)] = temp_dict
    return ret_dict

def main():
    old_stats = getStats()
    names = ["user", "nice", "system", "idle", "iowait", "irq", "softirq"]
    while (True):
        time.sleep(1)
        new_stats = getStats()
        print("_" * 60)
        print(("{:7s}" * 8).format("cpu", *names))
        for cpu in new_stats:
            res_list = []
            old_total = float(sum(old_stats[cpu].values()))
            new_total = float(sum(new_stats[cpu].values()))
            for name in names:
                res_list.append(100.0 * (new_stats[cpu][name] - old_stats[cpu][name]) / (new_total - old_total))
            print((cpu + "{:7.1f}"*7).format(*res_list))
        old_stats = new_stats

if __name__ == '__main__':
    main()
    
