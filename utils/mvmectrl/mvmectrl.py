
import socket
import argparse
import time
import sys

svr_ip_addr = "127.0.0.1" # change with -i option
svr_tcp_port = 50010      # change with -p option

RXBUFFER = 4096

class MvmeCtrl():

   def __init__(self, ip, port, verbose):
      self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
      self.s.connect((ip, port))
      self.verbose = verbose

   def send(self, data):
      if self.verbose:
         print("send: %s" %(data))
      self.s.send(data)

   def recv(self):
      rx = self.s.recv(RXBUFFER)
      if self.verbose:
         print("recv: %s" %(rx))
      return rx

   def startDaq(self):
      self.send('{ "id": "1", "jsonrpc": "2.0", "method": "startDAQ" }')
      rx = self.recv()

   def stopDaq(self):
      self.send('{ "id": "1", "jsonrpc": "2.0", "method": "stopDAQ" }')
      rx = self.recv()

if __name__ == '__main__':
   parser = argparse.ArgumentParser()
   parser.add_argument("-i", metavar='ipaddr', help = "server ip address (default %s)" % (svr_ip_addr), type = str)
   parser.add_argument("-p", metavar='port', help = "server tcp port (default %d)" % (svr_tcp_port), type = int)
   parser.add_argument("-c", metavar='cmd', help = "command (startDaq, stopDaq)", type = str)
   parser.add_argument("-v", help = "add debug prints", action='store_true')
   args = parser.parse_args()

   if args.i != None:
      svr_ip_addr = args.i

   if args.p != None:
      svr_tcp_port = args.p

   if args.c != None:
      ctrl = MvmeCtrl(svr_ip_addr, svr_tcp_port, args.v)

      if args.c == "startDaq":
         print("Starting Readout")
         ctrl.startDaq()
      elif args.c == "stopDaq":
         print("Stopping Readout")
         ctrl.stopDaq()
      else:
         print("Invalid command: %s" % (args.c))
   else:
      print("No command specified")
