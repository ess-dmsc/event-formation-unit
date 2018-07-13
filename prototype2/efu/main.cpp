/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/EFUArgs.h>
#include <common/StatPublisher.h>
#include <common/Trace.h>
#include <common/Version.h>
#include <efu/ExitHandler.h>
#include <efu/HwCheck.h>
#include <efu/Launcher.h>
#include <efu/Loader.h>
#include <efu/Parser.h>
#include <efu/Server.h>
#include <iostream>
#include <libs/include/Timer.h>
#include <libs/include/gccintel.h>
#include <unistd.h> // sleep()
#include <vector>

#define ONE_SECOND_US 1000000U

/** Load detector, launch pipeline threads, then sleep until timeout or break */
int main(int argc, char *argv[]) {
  BaseSettings DetectorSettings;
  std::vector<ThreadCoreAffinitySetting> AffinitySettings;
  std::shared_ptr<Detector> detector;
  std::string DetectorName;
  GraylogSettings GLConfig;
  Loader loader;
  HwCheck hwcheck;

  { //Make sure that the EFUArgs instance is deallocated before the detector plugin is
    EFUArgs efu_args;
    if (EFUArgs::Status::EXIT == efu_args.parseFirstPass(argc, argv)) {
      return 0;
    }
    loader.loadPlugin(efu_args.getDetectorName());
    if (not loader.IsOk()) {
      efu_args.printHelp();
      return -1;
    }

    { // This is to prevent accessing unloaded memory in a (potentially) unloaded
      // plugin.
      auto CLIArgPopulator = loader.GetCLIParserPopulator();
      CLIArgPopulator(efu_args.CLIParser);
    }
    if (EFUArgs::Status::EXIT == efu_args.parseSecondPass(argc, argv)) {
      return 0;
    }
    efu_args.printSettings();
    DetectorSettings = efu_args.getBaseSettings();
    detector = loader.createDetector(DetectorSettings);
    AffinitySettings = efu_args.getThreadCoreAffinity();
    DetectorName = efu_args.getDetectorName();
    GLConfig = efu_args.getGraylogSettings();
  }

  hwcheck.setMinimumMTU(DetectorSettings.MinimumMTU);

  int keep_running = 1;

  ExitHandler::InitExitHandler();

#ifdef GRAYLOG
  Log::AddLogHandler(new GraylogInterface(GLConfig.address, GLConfig.port));
  Log::SetMinimumSeverity(Severity::Debug);
#endif

  GLOG_INF("Starting Event Formation Unit");
  GLOG_INF("Event Formation Unit version: " + efu_version());
  GLOG_INF("Event Formation Unit build: " + efu_buildstr());
  XTRACE(MAIN, ALW, "Starting Event Formation unit\n");
  XTRACE(MAIN, ALW, "Event Formation Software Version: %s\n",
         efu_version().c_str());
  XTRACE(MAIN, ALW, "Event Formation Unit build: %s\n", EFU_STR(BUILDSTR));

  if (hwcheck.checkMTU(hwcheck.defaultIgnoredInterfaces) == false) {
    XTRACE(MAIN, ERR, "MTU checks failed, exiting...\n");
    GLOG_ERR("MTU checks failed, exiting.");
    return 1;
  }

  if (DetectorSettings.StopAfterSec == 0) {
    XTRACE(MAIN, ALW, "Event Formation Unit Exit (Immediate)\n");
    GLOG_INF("Event Formation Unit Exit (Immediate)");
    return 0;
  }


  XTRACE(MAIN, ALW, "Launching EFU as Instrument %s\n", DetectorName.c_str());

  Launcher launcher(AffinitySettings);

  launcher.launchThreads(detector);

  StatPublisher metrics(DetectorSettings.GraphiteAddress, DetectorSettings.GraphitePort);

  Parser cmdParser(detector, keep_running);
  Server cmdAPI(DetectorSettings.CommandServerPort, cmdParser);

  Timer RunTimer, livestats;

  while (true) {
    //Do not allow immediate exits
    if (RunTimer.timeus() >= (uint64_t)ONE_SECOND_US / 10) {
      if (keep_running == 0) {
        XTRACE(INIT, ALW, "Application stop, Exiting...\n");
        detector->stopThreads();
        sleep(1);
        break;
      }
    }

    if (RunTimer.timeus() >=
        DetectorSettings.StopAfterSec * (uint64_t)ONE_SECOND_US) {
      XTRACE(MAIN, ALW, "Application timeout, Exiting...\n");
      GLOG_INF("Event Formation Unit Exiting (User timeout)");
      detector->stopThreads();
      sleep(1);
      break;
    }

    if (livestats.timeus() >= ONE_SECOND_US && detector != nullptr) {
      metrics.publish(detector);
      livestats.now();
    }

    cmdAPI.server_poll();
    ExitHandler::Exit DoExit = ExitHandler::HandleLastSignal();
    if (DoExit == ExitHandler::Exit::Exit) {
      keep_running = 0;
    }
    usleep(500);
  }

  if (detector.use_count() > 1) {
    XTRACE(MAIN, WAR,
           "There are more than 1 strong pointers to the detector. This "
           "application may crash on exit.\n");
  }
  detector.reset();

  return 0;
}
