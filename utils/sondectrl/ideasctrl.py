#!/usr/local/bin/python
import binascii
import socket
import struct
import argparse

svr_ip_addr = "127.0.0.1"
svr_tcp_port = 1033

RXBUFFER = 4096

class IdeasCtrl():

   def __init__(self, ip, port):
      self.ip = ip
      self.port = port
      self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
      self.s.connect((ip, port))
      self.pktno = 0
      self.version = 0

   def formatdata(self, version, sysno, pkttype, seq, pktcount, length):
      s = struct.Struct('!BBHIH')
      return s.pack((version << 5) + sysno, pkttype, (seq << 14) + pktcount, 0, length)

   def send(self, data):
      self.s.send(data)
      self.pktno += 1

   def recv(self):
      return self.s.recv(RXBUFFER)

   # Assumes address is 16 bit and value is 16 bits
   def writesystemregister(self, sysno, address, value):
       s = struct.Struct('!BBHIHHBH')
       pkttype = 0x10
       pktseq = 0
       datalen = 5
       return s.pack((self.version << 5) + sysno, pkttype, (pktseq << 14) + self.pktno, 0,
                      datalen, address, 2, value)


if __name__ == '__main__':
   parser = argparse.ArgumentParser()
   parser.add_argument("-i", metavar='ipaddr', help = "server ip address (default %s)" % (svr_ip_addr), type = str)
   parser.add_argument("-p", metavar='port', help = "server tcp port (default %d)" % (svr_tcp_port), type = int)
   args = parser.parse_args()

   if args.i != None:
      svr_ip_addr = args.i

   if args.p != None:
      svr_tcp_port = args.p

ctrl = IdeasCtrl(svr_ip_addr, svr_tcp_port)

system = 1
txdata = ctrl.formatdata(0, system, 0xD1, 0, 0xAAA, 2)
print("Size: %s" % (len(txdata)))
print("Sending data:  %s" % (binascii.hexlify(txdata)))
ctrl.send(txdata)
rxdata = ctrl.recv()
print("Received data: %s" % (binascii.hexlify(rxdata)))

txdata = ctrl.writesystemregister(system, 0xfedc, 0xaabb)
print("Size: %s" % (len(txdata)))
print("Sending data:  %s" % (binascii.hexlify(txdata)))
ctrl.send(txdata)
rxdata = ctrl.recv()
print("Received data: %s" % (binascii.hexlify(rxdata)))
