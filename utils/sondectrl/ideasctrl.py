#!/usr/local/bin/python
import binascii
import socket
import struct
import argparse

svr_ip_addr = "127.0.0.1"
svr_tcp_port = 1033

RXBUFFER = 4096

class IdeasCtrl():
   # packet types
   cmd_read_register_request = 0x11

   #
   pkt_stand_alone = 0x00

   def __init__(self, ip, port):
      self.ip = ip
      self.port = port
      self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
      self.s.connect((ip, port))
      self.pktno = 0
      self.version = 0
      self.system = 0

   def send(self, data):
      self.s.send(data)
      self.pktno += 1

   def recv(self):
      return self.s.recv(RXBUFFER)

   def readsystemregister(self, sysno, address):
       s = struct.Struct('!BBHIH H')
       r4 = struct.Struct('!BBHIH HBI')
       r2 = struct.Struct('!BBHIH HBH')
       r1 = struct.Struct('!BBHIH HBB')
       pkttype = self.cmd_read_register_request
       pktseq = self.pkt_stand_alone
       datalen = 2
       self.send(s.pack((self.version << 5) + sysno, pkttype, (pktseq << 14) + self.pktno, 0,
                    datalen, address))
       rx = self.recv()
       #print("Received data: %s (len %d)" % (binascii.hexlify(rx), len(rx)))
       if len(rx) == 17:
          res = r4.unpack(rx)[7]
       elif len(rx) == 15:
          res = r2.unpack(rx)[7]
       elif len(rx) == 14:
          res = r1.unpack(rx)[7]
       else:
          res = "unsupported"
       return res

   # Assumes address is 16 bit and value is 16 bits
   def writesystemregister8(self, sysno, address, value):
       s = struct.Struct('!BBHIH HBH')
       pkttype = 0x10
       pktseq = self.pktno
       datalen = 4
       return s.pack((self.version << 5) + sysno, pkttype, (pktseq << 14) + self.pktno, 0,
                      datalen, address, 2, value)

   # Assumes address is 16 bit and value is 16 bits
   def writesystemregister16(self, sysno, address, value):
       s = struct.Struct('!BBHIH HBH')
       pkttype = 0x10
       pktseq = 0
       datalen = 5
       return s.pack((self.version << 5) + sysno, pkttype, (pktseq << 14) + self.pktno, 0,
                      datalen, address, 2, value)

   # Assumes address is 16 bit and value is 16 bits
   def writesystemregister32(self, sysno, address, value):
       s = struct.Struct('!BBHIH HBI')
       pkttype = 0x10
       pktseq = 0
       datalen = 7
       return s.pack((self.version << 5) + sysno, pkttype, (pktseq << 14) + self.pktno, 0,
                      datalen, address, 4, value)

#
# High level user functions
#

   def printregister(self, name, address):
      res = ctrl.readsystemregister(self.system, address)
      print("%-30s 0x%x  (%d)" % (name, res, res))

   def dumpallregisters(self):
      print("System")
      self.printregister("Serial Number",               0x0000)
      self.printregister("Firmware Type",               0x0001)
      self.printregister("Firmware Version",            0x0002)
      self.printregister("System Number",               0x0010)
      print("Cal Pulse Gen")
      self.printregister("Calibration Execute",         0x0c00)
      self.printregister("Calibration Pulse Polarity",  0x0c01)
      self.printregister("Calibration Num Pulses",      0x0c02)
      self.printregister("Calibration Pulse Length",    0x0c03)
      self.printregister("Calibration Pulse Interval",  0x0c04)
      print("DACs")
      self.printregister("Cal DAC",                     0x0e00)
      print("VATA readout")
      self.printregister("cfg_phystrig_en",             0xf016)
      self.printregister("cfg_forced_en",               0xf017)
      self.printregister("cfg_event_num",               0xf018)
      self.printregister("cfg_forced_asic",             0xf019)
      self.printregister("cfg_forced_channel",          0xf01a)
      self.printregister("cfg_timing_readout_en",       0xf020)
      self.printregister("cfg_event_num",               0xf021)
      self.printregister("cfg_all_ch_en",               0xf030)


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


ctrl.dumpallregisters()
