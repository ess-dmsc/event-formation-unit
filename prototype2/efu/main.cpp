/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/EFUArgs.h>
#include <common/StatPublisher.h>
#include <common/Trace.h>
#include <common/Version.h>
#include <efu/ExitHandler.h>
#include <efu/Launcher.h>
#include <efu/Parser.h>
#include <efu/Server.h>
#include <iostream>
#include <libs/include/Timer.h>
#include <libs/include/gccintel.h>
#include <unistd.h> // sleep()
#include <vector>
#include <efu/Loader.h>

#define ONE_SECOND_US 1000000U

/** Load detector, launch pipeline threads, then sleep until timeout or break */
int main(int argc, char *argv[]) {
  EFUArgs efu_args;
  if (not efu_args.parseAndProceed(argc, argv)) {
    return 0;
  }
  
  Loader loader(efu_args.getDetectorName());
  
  if (not loader.IsOk()) {
    GLOG_CRI("Detector not loadable, no processing ...");
    XTRACE(MAIN, CRI, "Detector not loadable, no processing ...\n");
    efu_args.printHelp();
    return -1;
  }
  
  std::shared_ptr<Detector> detector = loader.createDetector();
  
  int keep_running = 1;

  ExitHandler::InitExitHandler(&keep_running);

#ifdef GRAYLOG
  Log::AddLogHandler(
      new GraylogInterface(efu_args.graylog_ip, efu_args.graylog_port));
  Log::SetMinimumSeverity(Severity::Debug);
#endif
  
  efu_args.printSettings();
  GLOG_INF("Starting Event Formation Unit");
  GLOG_INF("Event Formation Unit version: " + efu_version());
  GLOG_INF("Event Formation Unit build: " + efu_buildstr());
  XTRACE(MAIN, ALW, "Starting Event Formation unit\n");
  XTRACE(MAIN, ALW, "Event Formation Software Version: %s\n", efu_version().c_str());
  XTRACE(MAIN, ALW, "Event Formation Unit build: %s\n", EFU_STR(BUILDSTR));

  if (efu_args.stopafter == 0) {
    XTRACE(MAIN, ALW, "Event Formation Unit Exit (Immediate)\n");
    GLOG_INF("Event Formation Unit Exit (Immediate)");
    return 0;
  }
  
  

  XTRACE(MAIN, ALW, "Launching EFU as Instrument %s\n", efu_args.det.c_str());

//  Loader loader(efu_args.det, (void *)&efu_args);
//  efu_args.detectorif = loader.createDetector();

  std::vector<int> cpus = {efu_args.cpustart, efu_args.cpustart + 1,
                           efu_args.cpustart + 2};

  Launcher(detector.get(), cpus);

  StatPublisher metrics(efu_args.graphite_ip_addr, efu_args.graphite_port);

  Parser cmdParser;
  Server cmdAPI(efu_args.cmdserver_port, cmdParser);

  Timer stop_timer, stop_cmd, livestats;

  while (1) {
    if (stop_cmd.timeus() >= (uint64_t)ONE_SECOND_US/10) {
      if (keep_running == 0 || efu_args.proc_cmd == efu_args.thread_cmd::EXIT) {
        XTRACE(INIT, ALW, "Application stop, Exiting...\n");
        efu_args.proc_cmd = efu_args.thread_cmd::THREAD_TERMINATE;
        sleep(2);
        return 1;
      }
    }

    if (stop_timer.timeus() >= (uint64_t)efu_args.stopafter * (uint64_t)ONE_SECOND_US) {
      XTRACE(MAIN, ALW, "Application timeout, Exiting...\n");
      GLOG_INF("Event Formation Unit Exiting (User timeout)");
      efu_args.proc_cmd = efu_args.thread_cmd::THREAD_TERMINATE;
      sleep(2);
      return 0;
    }

    if (livestats.timeus() >= ONE_SECOND_US && detector != nullptr) {
      metrics.publish(detector);
      livestats.now();
    }

    cmdAPI.server_poll();

    usleep(1000);
  }

  return 0;
}
