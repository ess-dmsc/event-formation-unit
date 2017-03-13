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
        self.x = 8
        self.y = 48
        self.z = 16
        self.clear()

    def coords(self, pixel):
        x = (pixel - 1) / (self.y * self.z)
        y = (pixel - 1) / self.z % self.y
        z = (pixel - 1) % self.z
        return [x,y,z]

    def addpixels(self, pixel):
        x,y,z = self.coords(pixel)

        #print("pixel: %d, x, y, z: %d, %d, %d\n" % (pixel, x, y, z))
        for i in range(len(x)):
            self.xy[y[i],x[i]] += 1
            self.zy[y[i],z[i]] += 1
            self.xz[self.z - z[i] - 1,x[i]] += 1

    def addpixel(self, pixel):
        x,y,z = self.coords(pixel)

        #print("pixel: %d, x, y, z: %d, %d, %d\n" % (pixel, x, y, z))

        self.xy[y,x] += 1
        self.zy[y,z] += 1
        self.xz[self.z - z - 1,x] += 1

    def clear(self):
        self.xy = numpy.zeros((self.y, self.x))
        self.zy = numpy.zeros((self.y, self.z))
        self.xz = numpy.zeros((self.z, self.x))

    def plot(self, title):
        pl.ion()
        pl.clf()

        plt.suptitle(title)
        plt.subplot(1,3,1)
        plt.title("x-y")
        plt.imshow(self.xy, interpolation="none")
        plt.colorbar()

        plt.subplot(1,3,2)
        plt.title("z-y")
        plt.imshow(self.zy, interpolation="none")
        plt.colorbar()

        plt.subplot(1,3,3)
        plt.title("x-z")
        plt.imshow(self.xz, interpolation="none")
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
    consumer = topic.get_simple_consumer(fetch_message_max_bytes = 1024 * 1024 * 50,
       consumer_group=codecs.encode(args.t, "utf-8"), auto_offset_reset=OffsetType.LATEST,
       reset_offset_on_start=True, consumer_timeout_ms=50)

    print("Starting main loop")

    clearplotafter = 5
    while (True):
        try:
            msg = consumer.consume(block = True)
            if (msg != None):
                a = bytearray(msg.value)
                arr = EventMessage.GetRootAsEventMessage(a, 0)
                print(arr.MessageId())
                print(arr.PulseTime())
                print(arr.DetectorIdLength())

                pixels_raw = arr.DetectorId_as_numpy_array()
                pixels = pixels_raw.view(numpy.uint32)

                proj.addpixels(pixels)

                proj.plot("events")
                proj.clear()
            else:
                pass
        except KeyboardInterrupt:
            break

if __name__ == "__main__":
    main()
