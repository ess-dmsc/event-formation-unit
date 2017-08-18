#!/usr/bin/python

import cmd, sys, os
from SocketDriver import SimpleSocket
import argparse

svr_ip_addr = "127.0.0.1"
svr_tcp_port = 8888



class EFUShell(cmd.Cmd):
   intro = "Event Formation Unit  Shell"
   prompt = '(efushell) '

   def __init__(self, ip, port):
      cmd.Cmd.__init__(self)
      self.ip = ip
      self.port = port
      self.driver = SimpleSocket(self.ip, self.port)

#
# EFU Commands
#

   def do_cspec_load_calib(self, line):
       'Load wire and grid calibrations from file'
       res = self.driver.Ask('CSPEC_LOAD_CALIB ' + line)
       print res

   def do_cspec_show_calib(self, line):
       'Load wire and grid calibrations from file'
       res = self.driver.Ask('CSPEC_SHOW_CALIB ' + line)
       print res

   def do_stat_getcount(self, line):
      'Get number of registered counters'
      res = self.driver.Ask('STAT_GET_COUNT')
      print res

   def do_stat_get(self, line):
      'Get specific counter'
      res = self.driver.Ask('STAT_GET ' + line)
      print res

   def do_version_get(self, line):
      'Get version and build information'
      res = self.driver.Ask('VERSION_GET ' + line)
      print res

   def do_detector_get(self, line):
      'Get detector information'
      res = self.driver.Ask('DETECTOR_INFO_GET')
      print res

#
# CMD behavior customization
#
   def do_EOF(self, line):
      print
      return True

   def do_shell(self, line):
      'Run a shell command'
      output = os.popen(line).read()
      print output

   def emptyline(self):
      print self.prompt

if __name__ == '__main__':
   parser = argparse.ArgumentParser()
   parser.add_argument("-i", metavar='ipaddr', help = "server ip address (default 127.0.0.1)", type = str)
   parser.add_argument("-p", metavar='port', help = "server tcp port (default 8888)", type = int)
   args = parser.parse_args()

   if args.i != None:
      svr_ip_addr = args.i

   if args.p != None:
      svr_tcp_port = args.p

   EFUShell(svr_ip_addr, svr_tcp_port).cmdloop()
