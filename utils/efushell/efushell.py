#!/usr/bin/python

import cmd, sys, os
from SocketDriver import SimpleSocket

class EFUShell(cmd.Cmd):

   intro = "Event Formation Unit  Shell"
   prompt = '(efushell) '
   ip = "127.0.0.1"
   port = 8888
   driver = SimpleSocket(ip, port)

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
   EFUShell().cmdloop()
