import json
import subprocess
import time
import sys
import os
from testutils import run_efu, run_data_generator_timed, get_metrics

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
from utils.efushell.EFUMetrics import Metrics


def get_stat(stat_name):
    return get_stats([stat_name])[stat_name]


def get_stats(stat_name_list):
    metrics = get_metrics()
    stats = {}
    for stat_name in stat_name_list:
        stats[stat_name] = metrics.return_metric(stat_name)
    return(stats)

def efu_drop_packets_check(efu, generator, module, throttle):
    time_s = 30
    readouts_per_packet = 100
    run_data_generator_timed(efu, generator, throttle, readouts_per_packet, time_s)
    time.sleep(5)
    new_packets_dropped = get_stat(f"efu.{module}.0.receive.dropped")
    return new_packets_dropped 


def assess_performance(efu, generator, module, throttle):
    time_s = 30
    readouts_per_packet = 100
    stat_name_list = [f"efu.{module}.0.receive.packets", f"efu.{module}.0.readouts.count", f"efu.{module}.0.events.count"]
    stats = get_stats(stat_name_list)
    run_data_generator_timed(efu, generator, throttle, readouts_per_packet, time_s)
    new_stats = get_stats(stat_name_list)
    for stat_name in stat_name_list:
        stats[stat_name] = (new_stats[stat_name] - stats[stat_name]) / time_s
    print(f"the following stats are per second, calculated over {time_s} seconds")
    print(stats)



def bisect_throttle_settings(efu, generator, module, throttle):
    packets_dropped = 0
    min_throttle = 0
    max_throttle = 1000
    for x in range(10):
        new_packets_dropped = efu_drop_packets_check(efu, generator, module, throttle)
        if new_packets_dropped == packets_dropped:
            print(f"Passed at throttle: {throttle}")
            max_throttle = throttle
            throttle = (throttle + min_throttle) / 2
        else:
            print(f"Failed at throttle: {throttle}")
            min_throttle = throttle
            throttle = (throttle + max_throttle) / 2
        packets_dropped = new_packets_dropped
        if((max_throttle - min_throttle) < 1):
            break
        print(f"New throttle to test: {throttle}")
        print(f"Min throttle: {min_throttle}, Max throttle: {max_throttle}")
        print(f"Packets dropped: {packets_dropped}")
    print(f"Best throttle achieved: {max_throttle}")
    best_passing_throttle = max_throttle
    return best_passing_throttle

def run_performance_test():
    efu = "./event-formation-unit"
    efu = "./build"

    with open('./test/performancetest.json') as f:
        data = json.load(f)

    for test in data['Tests']:
        efu_process = run_efu(efu, test['Module'], test['Config'])
        try:
            best_throttle = bisect_throttle_settings(efu, test['Generator'], test['Module'], test['InitThrottle'])
            assess_performance(efu, test['Generator'], test['Module'], best_throttle)
        except:
           efu_process.kill()
           raise
        efu_process.kill()
        time.sleep(5)

if __name__ == "__main__":
	run_performance_test()