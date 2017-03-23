#!/usr/bin/python
# -*- coding: utf-8 -*-

from pykafka import KafkaClient
from pykafka.common import OffsetType
from schemas.MonitorMessage import MonitorMessage
from schemas.DataField import DataField
from schemas.GEMHist import GEMHist
from schemas.GEMTrack import GEMTrack
import pylab as pl
import matplotlib.pyplot as plt
import argparse, codecs, numpy

xtrack = numpy.zeros((50, 128))
ytrack = numpy.zeros((50, 128))
xhist = numpy.zeros(1500)
yhist = numpy.zeros(1500)

def Plotit():
    pl.ion()
    pl.clf()
    global xhist, yhist, xtrack, ytrack
    plt.figure(1)
    plt.suptitle("Detector Monitor")
    ax1 = plt.subplot(2,1,1)
    ax2 = plt.subplot(2,1,2)
    ax1.set_title("x-strips - " + str(numpy.sum(xhist)) + " counts")
    ax2.set_title("y-strips - " + str(numpy.sum(yhist)) + " counts")
    ax1.bar(numpy.arange(1500), xhist, 1.0, color='r')
    ax2.bar(numpy.arange(1500), yhist, 1.0, color='r')

    plt.figure(2)
    ax3 = plt.subplot(2,1,1)
    ax3.set_title("X-Tracks")
    plt.imshow(xtrack, interpolation="none")
    ax4 = plt.subplot(2,1,2)
    ax4.set_title("Y-Tracks")
    plt.imshow(ytrack, interpolation="none")

    #plt.show()
    pl.pause(0.0001)

#
def updateGemTrack(gemtrackdata):
    global xtrack, ytrack
    xtrack = numpy.zeros((50, 128))
    ytrack = numpy.zeros((50, 128))

    x = GEMTrack()
    x.Init(gemtrackdata.Bytes, gemtrackdata.Pos)

    for i in range(x.XtrackLength()):
        #print("x: strip: %d, time: %d, adc: %d" % (x.Xtrack(i).Strip(), x.Xtrack(i).Time(), x.Xtrack(i).Adc()))
        xtrack[x.Xtrack(i).Time()][x.Xtrack(i).Strip()] = x.Xtrack(i).Adc()

    for i in range(x.YtrackLength()):
        #print("y: strip: %d, time: %d, adc: %d" % (x.Ytrack(i).Strip(), x.Ytrack(i).Time(), x.Ytrack(i).Adc()))
        ytrack[x.Ytrack(i).Time()][x.Ytrack(i).Strip()] = x.Ytrack(i).Adc()


#
def updateGemHist(gemhistdata):
    global xhist, yhist
    x = GEMHist()
    x.Init(gemhistdata.Bytes, gemhistdata.Pos)
    xhist = x.Xhist_as_numpy_array()
    yhist = x.Yhist_as_numpy_array()

    #print(xhist[80:100])



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
    consumer = topic.get_simple_consumer()
                    #  fetch_message_max_bytes = 1024 * 1024 * 50,
                    #  consumer_group=codecs.encode(envtopic, "utf-8"),
                    #  auto_offset_reset=OffsetType.LATEST,
                    #  reset_offset_on_start=True,
                    #  consumer_timeout_ms=50)

    print("Starting main loop")
    while (True):
        try:
            msg = consumer.consume(block = True)
            if (msg != None):
                a = bytearray(msg.value)
                arr = MonitorMessage.GetRootAsMonitorMessage(a, 0)
                if arr.DataType() == DataField.GEMHist:
                    updateGemHist(arr.Data())
                elif arr.DataType() == DataField.GEMTrack:
                    updateGemTrack(arr.Data())
                Plotit()
            else:
                pass
        except KeyboardInterrupt:
            break

if __name__ == "__main__":
    main()
