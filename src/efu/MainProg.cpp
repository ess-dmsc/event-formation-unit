// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Wrapper for EFU main application
//===----------------------------------------------------------------------===//

#include <common/StatPublisher.h>
#include <common/Version.h>
#include <common/debug/Log.h>
#include <cstdio>
#include <efu/ExitHandler.h>
#include <efu/Launcher.h>
#include <efu/MainProg.h>
#include <efu/Parser.h>
#include <efu/Server.h>

MainProg::MainProg(std::string instrument, int argc, char *argv[]) {

  if (Args.parseArgs(argc, argv) != EFUArgs::Status::CONTINUE) {
    exit(0);
  }
  Args.printSettings();
  DetectorSettings = Args.getBaseSettings();
  DetectorSettings.DetectorName = instrument;

  graylog.AddLoghandlerForNetwork(
      instrument, Args.getLogFileName(), Args.getLogLevel(),
      Args.getGraylogSettings().address, Args.getGraylogSettings().port);

  DetectorSettings.GraphitePrefix = std::string("efu.") + instrument;
}

int MainProg::run(Detector *inst) {
  detector = std::shared_ptr<Detector>(inst);
  int keep_running = 1;

  ExitHandler::InitExitHandler();

  int64_t statUpTime{0};
  Statistics mainStats;
  mainStats.setPrefix(DetectorSettings.GraphitePrefix,
                      DetectorSettings.GraphiteRegion);
  mainStats.create("main.uptime", statUpTime);

  LOG(MAIN, Sev::Info, "Starting Event Formation Unit");
  LOG(MAIN, Sev::Info, "Event Formation Unit version: {}", efu_version());
  LOG(MAIN, Sev::Info, "Event Formation Unit build: {}", efu_buildstr());

  if (DetectorSettings.NoHwCheck) {
    LOG(MAIN, Sev::Warning, "Skipping HwCheck - performance might suffer");
  } else {
    if (hwcheck.checkMTU(hwcheck.IgnoredInterfaces) == false) {
      LOG(MAIN, Sev::Error, "MTU checks failed, for a quick fix, try");
      LOG(MAIN, Sev::Error,
          "sudo ifconfig eth0 mtu 9000 (change eth0 to match your system)");
      LOG(MAIN, Sev::Error, "exiting...");
      return -1;
    }
  }

  if (DetectorSettings.StopAfterSec == 0) {
    LOG(MAIN, Sev::Info, "Event Formation Unit Exit (Immediate)");
    return 0;
  }

  LOG(MAIN, Sev::Info, "Launching EFU as Instrument {}",
      DetectorSettings.DetectorName);

  Launcher launcher;

  launcher.launchThreads(detector);

  StatPublisher metrics(DetectorSettings.GraphiteAddress,
                        DetectorSettings.GraphitePort);

  Parser cmdParser(detector, mainStats, keep_running);
  Server cmdAPI(DetectorSettings.CommandServerPort, cmdParser);

  Timer livestats;

  while (true) {
    // Do not allow immediate exits
    if (RunTimer.timeus() >= MicrosecondsPerSecond / 10) {
      if (keep_running == 0) {
        LOG(MAIN, Sev::Info, "Application stop, Exiting...");
        detector->stopThreads();
        sleep(1);
        break;
      }
    }

    if (RunTimer.timeus() >=
        DetectorSettings.StopAfterSec * MicrosecondsPerSecond) {
      LOG(MAIN, Sev::Info, "Application timeout, Exiting...");
      detector->stopThreads();
      sleep(1);
      break;
    }

    if (livestats.timeus() >= MicrosecondsPerSecond) {
      statUpTime = RunTimer.timeus() / 1000000;
      metrics.publish(detector, mainStats);
      livestats.reset();
    }

    cmdAPI.serverPoll();
    ExitHandler::Exit DoExit = ExitHandler::HandleLastSignal();
    if (DoExit == ExitHandler::Exit::Exit) {
      keep_running = 0;
    }
    usleep(500);
  }

  return 0;
}
