
import binascii
import socket
import struct
import argparse
import time

# 0070   00 c0 00 34 00 00 00 00 00 30 02 01 64 00 00 00  ...4.....0..d...
# 0080   20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20
# 0090   20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20
# 00a0   20 20 20 20 00 00 00 00 00 80
#                                      00 c0 00 35 00 00      .........5..
# 00b0   00 00 00 30 03 01 64 00 00 00 20 20 20 20 20 20  ...0..d...
# 00c0   20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20
# 00d0   20 20 20 20 20 20 20 20 20 20 20 20 20 20 00 00                ..
# 00e0   00 00 00 80                                      ....

svr_ip_addr = "127.0.0.1"
svr_tcp_port = 50010

RXBUFFER = 4096

asicscf1_bits = 356
asiccfg1 = [
  0x00, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80
]


registers = {'Serial Number': 0x0000,
             'Firmware Type': 0x0001,
             'Firmware Version': 0x0002,
             'System Number': 0x0010,
             'Calibration Execute': 0x0c00,
             'Calibration Pulse Polarity': 0x0c01,
             'Calibration Num Pulses': 0x0c02,
             'Calibration Pulse Length': 0x0c03,
             'Calibration Pulse Interval': 0x0c04,
             'Cal DAC': 0x0e00,
             'cfg_phystrig_en': 0xf016,
             'cfg_forced_en': 0xf017,
             'cfg_event_num': 0xf018,
             'cfg_forced_asic': 0xf019,
             'cfg_forced_channel': 0xf01a,
             'cfg_timing_readout_en': 0xf020,
             'cfg_event_num': 0xf021,
             'cfg_all_ch_en': 0xf030
            }

class IdeasCtrl():
   # packet types
   class cmd():
      WriteSystemRegister = 0x10
      ReadSystemRegister = 0x11
      SystemRegisterReadBack = 0x12
      ASICConfigRegisterWrite = 0xc0

   class seqflag():
      StandAlone = 0x00
      FirstPacket = 0x01
      ContinuationPacket = 0x02
      LastPacket = 0x03

   class asic():
       id0 = 0
       id1 = 1
       id2 = 2
       id3 = 3

   def __init__(self, ip, port, verbose):
      self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
      self.s.connect((ip, port))
      self.pktno = 0
      self.version = 0
      self.system = 0
      self.verbose = verbose

   def send(self, data):
      if self.verbose:
         print("send: %s" %(binascii.hexlify(data)))
      self.s.send(data)
      self.pktno += 1

   def recv(self):
      rx = self.s.recv(RXBUFFER)
      if self.verbose:
         print("recv: %s" %(binascii.hexlify(rx)))
      return rx



   def readsystemregister(self, address):
       s = struct.Struct('!BBHIH H')
       r4 = struct.Struct('!BBHIH HBI')
       r2 = struct.Struct('!BBHIH HBH')
       r1 = struct.Struct('!BBHIH HBB')
       pkttype = self.cmd.ReadSystemRegister
       pktseq = self.seqflag.StandAlone
       datalen = 2
       self.send(s.pack((self.version << 5) + self.system, pkttype, (pktseq << 14) + self.pktno, 0,
                    datalen, address))
       rx = self.recv()
       if len(rx) == 17:
          res = r4.unpack(rx)[7]
       elif len(rx) == 15:
          res = r2.unpack(rx)[7]
       elif len(rx) == 14:
          res = r1.unpack(rx)[7]
       else:
          print("Unsupported length %d" % (len(rx)))
          res = -1
       time.sleep(0.1)
       return res

   def writesystemregister(self, fmt, datalen, name, value):
       s = struct.Struct('!BBHIH HB' + fmt)
       pkttype = self.cmd.WriteSystemRegister
       pktseq = self.seqflag.StandAlone
       self.send(s.pack((self.version << 5) + self.system, pkttype, (pktseq << 14) + self.pktno, 0,
                        datalen, registers[name], datalen - 3, value))
       rx = self.recv()
       time.sleep(0.1)

   def writesystemregister8(self, address, value):
       self.writesystemregister('B', 4, address, value)

   def writesystemregister16(self, address, value):
       self.writesystemregister('H', 5, address, value)

   def writesystemregister32(self, address, value):
       self.writesystemregister('I', 7, address, value)

   def printregister(self, name):
      res = ctrl.readsystemregister(registers[name])
      print("%-30s 0x%x  (%d)" % (name, res, res))

#
# High level user functions
#
   def writeasicconf(self, asicid, data, datalen_bits):
       datalen = len(data)
       pkttype = self.cmd.ASICConfigRegisterWrite
       pktseq = self.seqflag.StandAlone

       s = struct.Struct('!BBHIH BH')
       tx_hdr = s.pack((self.version << 5) + self.system, pkttype, (pktseq << 14) + self.pktno, 0,
                    datalen + 3, asicid, datalen_bits)

       tx_data = struct.pack('B' * len(data), *data)
       self.send(tx_hdr + tx_data)


   def setcalibrationparms(self, polarity, nb_pulses, pulse_length, pulse_interval):
      self.writesystemregister8('Calibration Pulse Polarity', polarity)
      self.writesystemregister16('Calibration Num Pulses', nb_pulses)
      self.writesystemregister32('Calibration Pulse Length', pulse_length)
      self.writesystemregister32('Calibration Pulse Interval', pulse_interval)


   def configandstart(self):
      self.writesystemregister8('cfg_timing_readout_en', 0)
      self.writesystemregister8('cfg_phystrig_en', 0)
      self.writesystemregister8('cfg_all_ch_en', 0)
      self.writeasicconf(self.asic.id0, asiccfg1, asicscf1_bits)
      self.writeasicconf(self.asic.id1, asiccfg1, asicscf1_bits)
      self.writeasicconf(self.asic.id2, asiccfg1, asicscf1_bits)
      self.writeasicconf(self.asic.id3, asiccfg1, asicscf1_bits)
      self.writesystemregister16('cfg_event_num', 250)
      self.writesystemregister8('cfg_timing_readout_en', 1)

   def stopreadout(self):
      self.writesystemregister8('cfg_timing_readout_en', 0)
      self.writesystemregister8('cfg_phystrig_en', 0)
      self.writesystemregister8('cfg_all_ch_en', 0)




   def dumpallregisters(self):
      print("System")
      self.printregister("Serial Number")
      self.printregister("Firmware Type")
      self.printregister("Firmware Version")
      self.printregister("System Number")
      print("Cal Pulse Gen")
      self.printregister("Calibration Execute")
      self.printregister("Calibration Pulse Polarity")
      self.printregister("Calibration Num Pulses")
      self.printregister("Calibration Pulse Length")
      self.printregister("Calibration Pulse Interval")
      print("DACs")
      self.printregister("Cal DAC")
      print("VATA readout")
      self.printregister("cfg_phystrig_en")
      self.printregister("cfg_forced_en")
      self.printregister("cfg_event_num")
      self.printregister("cfg_forced_asic")
      self.printregister("cfg_forced_channel")
      self.printregister("cfg_timing_readout_en")
      self.printregister("cfg_event_num")
      self.printregister("cfg_all_ch_en")


if __name__ == '__main__':
   parser = argparse.ArgumentParser()
   parser.add_argument("-i", metavar='ipaddr', help = "server ip address (default %s)" % (svr_ip_addr), type = str)
   parser.add_argument("-p", metavar='port', help = "server tcp port (default %d)" % (svr_tcp_port), type = int)
   parser.add_argument("-v", help = "add debug prints", action='store_true')
   args = parser.parse_args()

   if args.i != None:
      svr_ip_addr = args.i

   if args.p != None:
      svr_tcp_port = args.p

ctrl = IdeasCtrl(svr_ip_addr, svr_tcp_port, args.v)


ctrl.configandstart()

#ctrl.writeasicconf(ctrl.asic.id1, asiccfg1, asicscf1_bits)


# ctrl.dumpallregisters()
# ctrl.setcalibrationparms(1,10,11,512)
#
# print("Doing it all again")
#
# ctrl.dumpallregisters()
# ctrl.setcalibrationparms(0,1,1,500)
