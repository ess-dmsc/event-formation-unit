#!/usr/bin/python


from SocketDriver import SimpleSocket
import curses, sys, os, time

driver = SimpleSocket("localhost", 8888)

def printstats():
    while (1):
      print(chr(27) + "[2J" + chr(27) + "[;H")
      print("="*73)
      print("Input Counters           | Processing Counters        | Output Counters")
      print("-"*73)
      ires = driver.Ask('STAT_INPUT').split()
      irxpkt = int(ires[1])
      irxbytes = int(ires[2])
      ipusherr = int(ires[3])

      pres = driver.Ask('STAT_PROCESSING').split()
      prxreadouts = int(pres[1])
      prxerrbytes = int(pres[2])
      prxdiscards = int(pres[3])
      pidle = int(pres[4])
      ppusherrs = int(pres[5])
      ppixerrs = int(pres[7])

      ores = driver.Ask('STAT_OUTPUT').split()
      oevents = int(ores[1])
      oidle = int(ores[2])
      obytes = int(ores[3])

      print("Rx Packets %10d    | Evt Readouts %10d    | Events %10d" % (irxpkt, prxreadouts, oevents))
      print("Rx Bytes   %10d    | Evt Discards %10d    | Bytes  %10d" % (irxbytes, prxdiscards, obytes))
      print("                         | Evt Pixerrs  %10d    |" % (ppixerrs))
      print("                         | Evt Errbytes %10d    |" % (prxerrbytes))
      print("-"*73)
      print("Push Errs  %10d                  %10d     |" % (ipusherr, ppusherrs))
      print("Idle                                   %10d     |        %10d" % (pidle, oidle))
      print("-"*73)
      time.sleep(1)

printstats()
