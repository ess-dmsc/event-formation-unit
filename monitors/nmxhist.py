#!/usr/bin/python
# -*- coding: utf-8 -*-

from pykafka import KafkaClient
from pykafka.common import OffsetType
from schemas.MonitorMessage import MonitorMessage
from schemas.DataField import DataField
from schemas.GEMHist import GEMHist
import pylab as pl
import matplotlib.pyplot as plt
import argparse, codecs, numpy

#
def updateGemHist(gemhistdata):
    pl.ioff()
    pl.clf()
    plt.title("Strip histograms")
    ax1 = plt.subplot(2,1,1)
    ax2 = plt.subplot(2,1,2)

    x = GEMHist()
    x.Init(gemhistdata.Bytes, gemhistdata.Pos)
    xhist = x.Xhist_as_numpy_array()
    print(xhist[80:100])
    yhist = x.Yhist_as_numpy_array()
    pl.ion()
    ax1.bar(numpy.arange(1500), xhist, 1.0, color='r')
    ax2.bar(numpy.arange(1500), yhist, 1.0, color='r')
    pl.ioff()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-b", help = "Broker to connect to.", type = str)
    args = parser.parse_args()

    if (args.b == None):
        print("Broker must be given as argument.")
        exit(0)

    envtopic = "NMX_monitor"
    client = KafkaClient(hosts=args.b)
    topic = client.topics[codecs.encode(envtopic, "utf-8")]
    consumer = topic.get_simple_consumer(
                     fetch_message_max_bytes = 1024 * 1024 * 50,
                     consumer_group=codecs.encode(envtopic, "utf-8"),
                     auto_offset_reset=OffsetType.LATEST,
                     reset_offset_on_start=True,
                     consumer_timeout_ms=50)

    print("Starting main loop")
    while (True):
        try:
            msg = consumer.consume(block = True)
            if (msg != None):
                print("Got a message")
                a = bytearray(msg.value)
                arr = MonitorMessage.GetRootAsMonitorMessage(a, 0)
                if arr.DataType() == DataField.GEMHist:
                    print("GEMHist\n")
                    updateGemHist(arr.Data())
                plt.pause(0.01)
            else:
                pass
        except KeyboardInterrupt:
            break

if __name__ == "__main__":
    main()
