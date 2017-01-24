#!/usr/bin/python


from SocketDriver import SimpleSocket
import curses, sys, os, time, thread

driver = SimpleSocket("localhost", 8888)

global statclear

statclear = 0

def printstats(val0, val1):
    global statclear
    i = 0
    win = curses.newwin(13, 80, 0, 0)
    then = time.time()
    old_events = int(driver.Ask('STAT_OUTPUT').split()[1])
    maxevrate = 0
    while (1):
      if statclear:
          maxevrate = 0
          statclear = 0
      win.clear()
      win.border(0)
      curses.curs_set(0)
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

      now = time.time()
      evrate = (oevents - old_events)/(now - then)
      maxevrate = max(evrate, maxevrate)
      old_events = oevents
      then = now

      win.addstr(1,2, "Input Counters           | Processing Counters        | Output Counters")
      win.addstr(2,2, "-"*73)
      win.addstr(3,2, "Rx Bytes   %10d    | Evt Readouts %10d    | Bytes  %10d" % (irxbytes, prxreadouts, obytes))
      win.addstr(4,2, "Rx Packets %10d    | Evt Discards %10d    | Events %10d" % (irxpkt, prxdiscards, oevents))
      win.addstr(5,2, "                         | Evt Pixerrs  %10d    | Event/s %9d" % (ppixerrs, evrate))
      win.addstr(6,2, "                         | Evt Errbytes %10d    | Max     %9d" % (prxerrbytes, maxevrate))
      win.addstr(7,2, "-"*73)
      win.addstr(8,2, "Push Errs  %10d    |              %10d    |" % (ipusherr, ppusherrs))
      win.addstr(9,2, "Idle                     |              %10d    |        %10d" % (pidle, oidle))
      win.addstr(10,2, "-"*73)
      win.addstr(11,2, "Elapsed time: " + str(i))
      win.refresh()
      time.sleep(1)
      i += 1

def command(line):
    global statclear
    res = line.split()
    if len(res) <= 0:
        return ""

    cmd = res[0]
    ret = "Invalid command: " + cmd
    if cmd == "clear" or cmd == "c":
        ret = driver.Ask('STAT_RESET')
        statclear = 1

    if (cmd == "load" or cmd == "l") and len(res) == 2:
        ret = driver.Ask('CSPEC_LOAD_CALIB ' + res[1])

    if cmd == "quit" or cmd == "q":
        curses.nocbreak()
        curses.endwin()
        sys.exit(0)

    return ret

def helptext():
    bot.addstr(1,2, "Commands:")
    bot.addstr(2,2, "  clear        clear the counters")
    bot.addstr(3,2, "  load calib   load calibration file")
    bot.addstr(4,2, "  quit         quit application")
    bot.addstr(6,2, CMD)

basewin = curses.initscr()
thread.start_new_thread(printstats, (0, 0))

bot = curses.newwin(10, 120, 15, 0)
curses.echo()
curses.cbreak()
bot.keypad(1)
CMD = "cmd> "

line=""
bot.clear()
bot.addstr(1,2, CMD)

helptext()
while (1):
    event = bot.getch()
    try:
        c = chr(event)
    except:
        c = ''
    line += c

    bot.clear()
    helptext()
    if event == 10:
        if len(line) > 0:
            res = command(line)
            bot.addstr(7,2, res)
            line = ""
            bot.insertln()
    bot.addstr(6,2, CMD + line)
    bot.refresh()
