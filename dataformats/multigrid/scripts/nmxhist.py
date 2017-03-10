#!/usr/bin/python
# -*- coding: utf-8 -*-

from pykafka import KafkaClient
from pykafka.common import OffsetType
import argparse
import signal, time, sys
import struct, codecs
import pylab as pl
import numpy
import matplotlib.pyplot as plt

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-b", help = "Broker to connect to.", type = str)
    parser.add_argument("-t", help = "Topic to subscribe to.", type = str)
    args = parser.parse_args()

    if (args.b == None or args.t == None):
        print("Broker and topic must be given as arguments.")
        exit(0)

    client = KafkaClient(hosts=args.b)
    topic = client.topics[codecs.encode(args.t, "utf-8")]
    consumer = topic.get_simple_consumer(fetch_message_max_bytes = 1024 * 1024 * 50, consumer_group=codecs.encode(args.t, "utf-8"),
                     auto_offset_reset=OffsetType.LATEST,
                     reset_offset_on_start=True,
                     consumer_timeout_ms=50)

    print("Starting main loop")
    while (True):
        try:
            msg = consumer.consume(block = True)
            if (msg != None):
                a = bytearray(msg.value)
                #print("Len: %d" % (len(a)/4))
                xhist = numpy.zeros(1500)
                yhist = numpy.zeros(1500)
                for ix in range(len(a)/8):
                    i = ix * 4
                    xhist[ix] = a[i] + a[i + 1]*256 + a[i + 2]*256*256 +  a[i + 3]*256*256*256
                    i = (ix + 1500) * 4
                    yhist[ix] = a[i] + a[i + 1]*256 + a[i + 2]*256*256 +  a[i + 3]*256*256*256

                print xhist
                pl.ion()
                pl.clf()
                plt.title("Strip histograms")
                plt.subplot(2,1,1)
                plt.bar(numpy.arange(1500), numpy.array(xhist), 1.0, color='r')
                plt.subplot(2,1,2)
                plt.bar(numpy.arange(1500), numpy.array(yhist), 1.0, color='r')
                pl.pause(0.001)
            else:
                pass
        except KeyboardInterrupt:
            break

if __name__ == "__main__":
    main()
