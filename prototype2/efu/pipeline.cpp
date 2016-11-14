/** Copyright (C) 2016 European Spallation Source ERIC*/

#include <common/EFUArgs.h>
#include <common/Trace.h>
#include <efu/Launcher.h>
#include <efu/Server.h>
#include <iostream>
#include <libs/include/Timer.h>
#include <unistd.h> // sleep()
#include <vector>

/**
 * Load detector, launch pipeline threads, then sleep forever
 */
int main(int argc, char *argv[]) {

  EFUArgs opts(argc, argv);

  XTRACE(MAIN, ALW, "Launching EFU as Instrument %s\n", opts.det.c_str());

  Loader detector(opts.det);

  std::vector<int> cpus = {opts.cpustart, opts.cpustart + 1, opts.cpustart + 2};

  Launcher(&detector, &opts, cpus);

  Server cmdAPI(8888, opts);
  opts.stat.set_mask(opts.reportmask);

  Timer stop, report;
  while (1) {
    if (stop.timeus() >= opts.stopafter * 1000000LU) {
      sleep(2);
      XTRACE(MAIN, ALW, "Exiting...\n");
      exit(1);
    }

    if (report.timeus() >= 1000000U) {
      opts.stat.report();
      report.now();
    }

    cmdAPI.server_poll();

    usleep(1000);
  }

  return 0;
}
