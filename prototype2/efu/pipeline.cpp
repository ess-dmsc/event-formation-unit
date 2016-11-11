/** Copyright (C) 2016 European Spallation Source ERIC*/

#include <common/EFUArgs.h>
#include <common/Trace.h>
#include <efu/Launcher.h>
#include <iostream>
#include <libs/include/Timer.h>
#include <unistd.h> // sleep()
#include <vector>



/**
 * Load detector, launch pipeline threads, then sleep forever
 */
int main(int argc, char *argv[]) {

  EFUArgs opts(argc, argv);

  XTRACE(INIT, ALW, "Launching EFU as Instrument %s\n", opts.det.c_str());

  Loader dynamic(opts.det);

  std::vector<int> cpus = {opts.cpustart, opts.cpustart + 1, opts.cpustart + 2};

  //Launcher(&dynamic, &opts, 12, 13, 14);
  Launcher(&dynamic, &opts, cpus);

  Timer stop;
  while (stop.timeus() < opts.stopafter * 1000000LU) {

    opts.stat.report(opts.reportmask);

    sleep(1);
  }

  sleep(2);
  XTRACE(INIT, ALW, "Exiting...\n");
  exit(1);
  return 0;
}
