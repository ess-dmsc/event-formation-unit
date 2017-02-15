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

class Proj:
    def __init__(self):
        self.x = 256
        self.y = 256
        self.clear()

    def coords(self, pixel):
        y = (pixel - 1) / 256
        x = (pixel - 1) - y * 256
        return [x,y]

    def addpixel(self, pixel):
        x,y = self.coords(pixel)

        self.xy[y,x] += 1

    def clear(self):
        self.xy = numpy.zeros((self.y, self.x))

    def plot(self, title):
        pl.ion()
        pl.clf()

        plt.suptitle(title)
        plt.subplot(1,1,1)
        plt.title("x-y")
        plt.imshow(self.xy, interpolation="none")
        plt.colorbar()

        pl.pause(0.001)



def main():
    proj = Proj()

    parser = argparse.ArgumentParser()
    parser.add_argument("-b", help = "Broker to connect to.", type = str)
    parser.add_argument("-t", help = "Topic to subscribe to.", type = str)
    args = parser.parse_args()

    if (args.b == None or args.t == None):
        print("Broker and topic must be given as arguments.")
        exit(0)

    client = KafkaClient(hosts=args.b)
    topic = client.topics[codecs.encode(args.t, "utf-8")]
    consumer = topic.get_simple_consumer(fetch_message_max_bytes = 1024 * 1024 * 50, consumer_group=codecs.encode(args.t, "utf-8"), auto_offset_reset=OffsetType.LATEST, reset_offset_on_start=True, consumer_timeout_ms=50)

    print("Starting main loop")

    plotint = 1
    plotevery =  plotint
    accumulated = 0
    maxoffset = 0
    minoffset = 999999999
    plotrange = 10000
    while (True):
        try:
            msg = consumer.consume(block = True)
            if (msg != None):
                a = bytearray(msg.value)
                for i in range(plotrange):
                    pixel = a[i*8 + 5]*256 + a[i*8 + 4]
                    if  (pixel > 65535 or pixel < 0):
                       printf("Geometry error pixel %d\n" % (pixel))
                       sys.exit(1)
                    proj.addpixel(pixel)
                plotevery -= 1
                maxoffset = max(msg.offset, maxoffset)
                minoffset = min(msg.offset, minoffset)
                accumulated += (plotint * plotrange)
                if plotevery == 0:
                    proj.plot("events: " + str(plotint * plotrange) +
                    "(total: " + str(accumulated) + ")\noffset: " 
                    + str(minoffset) + "-" + str(maxoffset))
                    #proj.clear()
                    minoffset = 999999999
                    plotevery = plotint
            else:
                pass
        except KeyboardInterrupt:
            break

if __name__ == "__main__":
    main()
