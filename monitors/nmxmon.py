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
from schemas.EventMessage import EventMessage


class Proj(object):
    def __init__(self):
        self.x = 256
        self.y = 256
        self.clear()

    def coords(self, pixel):
        y = (pixel) / 256
        x = (pixel) - y * 256
        return [x,y]

    def addpixels(self, pixels):
        x,y = self.coords(pixels)
        for i in range(len(x)):
            self.xy[y[i],x[i]] += 1

    def clear(self):
        self.xy = numpy.zeros((self.y, self.x))

    def plot(self, title):
        pl.ion()
        pl.clf()

        plt.suptitle(title)
        plt.subplot(1,1,1)
        plt.title("x-y")
        plt.imshow(self.xy, interpolation="nearest")
        plt.colorbar()

        pl.pause(0.001)




def main():
    proj = Proj()

    parser = argparse.ArgumentParser()
    parser.add_argument("-b", help = "Broker to connect to.", type = str)
    args = parser.parse_args()

    if (args.b == None):
        print("Broker and topic must be given as arguments.")
        exit(0)

    envtopic = "NMX_detector"
    client = KafkaClient(hosts=args.b)
    topic = client.topics[codecs.encode(envtopic, "utf-8")]
    consumer = topic.get_simple_consumer(fetch_message_max_bytes = 1024 * 1024 * 50,
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
                arr = EventMessage.GetRootAsEventMessage(a, 0)
                print("pulse_time: %d" % (arr.PulseTime()))
                print("seqno: %d" % (arr.MessageId()))
                nbevents = arr.DetectorIdLength()
                print("events: %d" % (nbevents))

                pixels_raw = arr.DetectorId_as_numpy_array()
                pixels = pixels_raw.view(numpy.uint32)
                print(pixels[0:min(10, nbevents)])
                proj.clear()
                proj.addpixels(pixels)

                proj.plot("events")
                #proj.clear()
            else:
                pass
        except KeyboardInterrupt:
            break

if __name__ == "__main__":
    main()
