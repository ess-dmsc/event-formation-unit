import subprocess
import time
import sys
import os

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
from utils.efushell.EFUMetrics import Metrics

def run_efu(efu, module, config):
    print("Running EFU")
    efu_command = f"{efu}/bin/efu --det {efu}/modules/{module}.so --nohwcheck --file {config} --region 0 --graphite 127.0.0.1"
    print(efu_command)
    efu_process = subprocess.Popen(
        f"exec {efu_command}",
        shell=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )

    # waiting for EFU start up to finish (estimated 5 seconds is enough time)
    # checking if the EFU is still running after start up
    time.sleep(5)
    poll = efu_process.poll()
    if poll is not None:
        out, errs = efu_process.communicate()
        print(out)
        print(errs)
        raise Exception(f"Running efu with command {efu_command} failed")
    return efu_process


def run_data_generator(efu, generator, packets, throttle):
    print("Running Data Generator")
    generator_process = subprocess.Popen(
        f"{efu}/generators/{generator} -a {packets} -o 50 -t {int(throttle)}",
        shell=True,
    )
    exit_code = generator_process.wait()
    if exit_code != 0:
        raise Exception(f"Artififial generator {generator} failed")
    return generator_process

def run_data_generator_timed(efu, generator, throttle, readouts_per_packet, time_s):
    print("Running Data Generator")
    generator_process = subprocess.Popen(
        f"{efu}/generators/{generator} -l -o {readouts_per_packet} -t {int(throttle)}",
        shell=True,
    )
    time.sleep(time_s)
    generator_process.kill()
    return generator_process

def get_metrics():
    print("Getting EFU Stats")
    try:
        metrics = Metrics("127.0.0.1", 8888)
        metrics.get_all_metrics(metrics.get_number_of_stats())
        return metrics
    except:
        # socket errors can happen when attempting to get metrics if grafana connection is hanging
        print("failed to get metrics")
        raise
