/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/EFUArgs.h>
#include <common/StatPublisher.h>
#include <common/Trace.h>
#include <efu/Launcher.h>
#include <efu/Parser.h>
#include <efu/Server.h>
#include <iostream>
#include <libs/include/Timer.h>
#include <unistd.h> // sleep()
#include <vector>

#define ONE_SECOND_US 1000000U

/**
 * Load detector, launch pipeline threads, then sleep forever
 */
int main(int argc, char *argv[]) {

  efu_args = new EFUArgs(argc, argv);

  if (efu_args->stopafter == 0) {
    return 0;
  }

  XTRACE(MAIN, ALW, "Launching EFU as Instrument %s\n", efu_args->det.c_str());

  Loader loader(efu_args->det, (void *)efu_args);

  std::vector<int> cpus = {efu_args->cpustart, efu_args->cpustart + 1,
                           efu_args->cpustart + 2};

  Launcher(&loader, cpus);

  StatPublisher metrics(efu_args->graphite_ip_addr, efu_args->graphite_port);
  Parser cmdParser;
  Server cmdAPI(8888, cmdParser);

  Timer stop, livestats;
  while (1) {
    if (stop.timeus() >= efu_args->stopafter * ONE_SECOND_US) {
      sleep(2);
      XTRACE(MAIN, ALW, "Exiting...\n");
      exit(1);
    }

    if (livestats.timeus() >= ONE_SECOND_US) {
      metrics.publish(loader.detector);
      livestats.now();
    }

    cmdAPI.server_poll();

    usleep(1000);
  }

  return 0;
}
