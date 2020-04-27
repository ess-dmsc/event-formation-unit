import sys
from os import path
sys.path.append(path.join(path.dirname(path.dirname(path.abspath(__file__))), "utils", "efushell"))
from verifymetrics import verify_metric
from time import sleep


def test_efu_received_all_sent_udp_packets(docker_compose):
    sleep(500)  # Wait (some random amount) of time for the EFU to receive and process the event data
    res, name, op, value, retval = verify_metric("localhost", 8889, "efu.mbcaen.0.receive.packets:30870")
    assert res, "Tested metric {}: expected {} {}, got {}".format(name, op, value, retval)
