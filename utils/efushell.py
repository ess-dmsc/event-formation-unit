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

   def do_load_cspec_calib(self, line):
       'Load wire and grid calibrations from file'
       res = self.driver.Ask('LOAD_CSPEC_CALIB ' + line)
       print res

   def do_stat_clear(self, line):
      'Clear statistics'
      res = self.driver.Ask('STAT_RESET')
      print res

   def do_stat_mask_set(self, line):
      'Set reporting mask'
      res = self.driver.Ask('STAT_MASK_SET ' + line)
      print res

   def do_stat_get(self, line):
      'Clear statistics'
      res = self.driver.Ask('STAT_INPUT')
      print res
      res = self.driver.Ask('STAT_PROCESSING')
      print res
      res = self.driver.Ask('STAT_OUTPUT')
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
