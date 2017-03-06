/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/EFUArgs.h>
#include <common/StatPublisher.h>
#include <common/Trace.h>
#include <efu/ExitHandler.h>
#include <efu/Launcher.h>
#include <efu/Parser.h>
#include <efu/Server.h>
#include <iostream>
#include <libs/include/Timer.h>
#include <libs/include/gccintel.h>
#include <unistd.h> // sleep()
#include <vector>

#define ONE_SECOND_US 1000000U

/**
 * Load detector, launch pipeline threads, then sleep forever
 */
int main(int argc, char *argv[]) {

  ExitHandler exithandler;

  efu_args = new EFUArgs(argc, argv);

#ifdef GRAYLOG
  Log::AddLogHandler(
      new GraylogInterface(efu_args->graylog_ip, efu_args->graylog_port));
  Log::SetMinimumSeverity(Severity::Debug);
#endif
  GLOG_INF("Starting efu2");

  if (efu_args->stopafter == 0) {
    return 0;
  }

  XTRACE(MAIN, ALW, "Launching EFU as Instrument %s\n", efu_args->det.c_str());

  Loader loader(efu_args->det, (void *)efu_args);
  efu_args->detectorif = loader.detector;

  std::vector<int> cpus = {efu_args->cpustart, efu_args->cpustart + 1,
                           efu_args->cpustart + 2};

  Launcher(&loader, cpus);

  StatPublisher metrics(efu_args->graphite_ip_addr, efu_args->graphite_port);
  Parser cmdParser;
  Server cmdAPI(efu_args->cmdserver_port, cmdParser);

  Timer stop, livestats;
  while (1) {
    if (stop.timeus() >=
        (uint64_t)efu_args->stopafter * (uint64_t)ONE_SECOND_US) {
      sleep(2);
      XTRACE(MAIN, ALW, "Exiting...\n");
      exithandler.Exit();
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
