import sys
from os import path
sys.path.append(path.join(path.dirname(path.dirname(path.abspath(__file__))), "utils", "efushell"))
from verifymetrics import verify_metrics


def test_efu_received_all_sent_udp_packets(docker_compose):
    for name, op, value, retval in verify_metrics("localhost", "8889", "efu.mbcaen.receive.packets:2000"):
        print("Validation failed for {}: expected {} {}, got {}".format(name, op, value, retval), flush=True)
        assert 0
