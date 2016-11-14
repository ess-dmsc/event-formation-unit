#!/usr/bin/python

import os, sys, time, threading, inspect

from SocketDriver import SimpleSocket


## Low level driver for TCPIP based queried
# do not edit this
class XenaSocketDriver(SimpleSocket):

    def __init__(self, hostname, port = 8888):
        SimpleSocket.__init__(self, hostname = hostname, port = port)
        SimpleSocket.set_keepalives(self)
        self.access_semaphor = threading.Semaphore(1)

    def SendCommand(self, cmd):
        self.access_semaphor.acquire()
        print "Sending command: " + cmd
        SimpleSocket.SendCommand(self, cmd)
        self.access_semaphor.release()

    def Ask(self, cmd):
        print "Sending command: " + cmd
        self.access_semaphor.acquire()
        reply = SimpleSocket.Ask(self, cmd).strip('\n')
        self.access_semaphor.release()
        return reply

driver= XenaSocketDriver("127.0.0.1")


res = driver.Ask("STAT_INPUT")
print("reply: %s" % (res))

res = driver.Ask("STAT_RESET")
print("reply: %s" % (res))

res = driver.Ask("STAT_MASK 0x3")
print("reply: %s" % (res))

res = driver.Ask("Hello")
print("reply: %s" % (res))
