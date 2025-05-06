// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Wrapper for EFU main application
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/StatPublisher.h>
#include <common/Version.h>
#include <efu/ExitHandler.h>
#include <efu/Launcher.h>
#include <efu/MainProg.h>
#include <efu/Parser.h>
#include <efu/Server.h>

MainProg::MainProg(const std::string &instrument, int argc, char *argv[]) {

  if (Args.parseArgs(argc, argv) != EFUArgs::Status::CONTINUE) {
    exit(0);
  }
  Args.printSettings();
  DetectorSettings = Args.getBaseSettings();
  DetectorSettings.DetectorName = instrument;

  // If KafkaTopic is set via CLI use that topic and generate the _samples
  // topic for the raw readout samples. Else use the default value

  if (DetectorSettings.KafkaTopic.empty()) {
    DetectorSettings.KafkaTopic = instrument + "_detector";
    DetectorSettings.KafkaDebugTopic = instrument + "_detector_samples";
  } else {
    DetectorSettings.KafkaDebugTopic = DetectorSettings.KafkaTopic + "_samples";
  }

  graylog.AddLoghandlerForNetwork(
      instrument, Args.getLogFileName(), Args.getLogLevel(),
      Args.getGraylogSettings().address, Args.getGraylogSettings().port);

  // Allow for customisation
  if (DetectorSettings.GraphitePrefix.empty()) {
    DetectorSettings.GraphitePrefix = std::string("efu.") + instrument;
  }
}

int MainProg::run(Detector *inst) {
  detector = std::shared_ptr<Detector>(inst);
  int keep_running = 1;

  ExitHandler::InitExitHandler();

  std::string Name{DetectorSettings.DetectorName};

  int64_t statUpTime{0};
  Statistics mainStats;
  mainStats.setPrefix(DetectorSettings.GraphitePrefix,
                      DetectorSettings.GraphiteRegion);
  mainStats.create("main.uptime", statUpTime);

  LOG(MAIN, Sev::Info, "Event Formation Unit ({}) Starting", Name);
  LOG(MAIN, Sev::Info, "Event Formation Unit ({}) version: {}", Name, efu_version());
  LOG(MAIN, Sev::Info, "Event Formation Unit ({}) build: {}", Name, efu_buildstr());

  if (DetectorSettings.NoHwCheck) {
    LOG(MAIN, Sev::Warning, "({}) Skipping HwCheck - performance might suffer", Name);
  } else {
    if (hwcheck.checkMTU(DetectorSettings.Interfaces) == false) {
      LOG(MAIN, Sev::Error, "({}) MTU checks failed, for a quick fix, try", Name);
      LOG(MAIN, Sev::Error,
          "sudo ifconfig eth0 mtu 9000 (change eth0 to match your system)");
      LOG(MAIN, Sev::Error, "exiting...");
      return -1;
    }
  }

  if (DetectorSettings.StopAfterSec == 0) {
    LOG(MAIN, Sev::Info, "Event Formation Unit ({}) Exit (Immediate)", Name);
    return 0;
  }

  LOG(MAIN, Sev::Info, "Launching EFU as Instrument {}",
      DetectorSettings.DetectorName);

  Launcher launcher(keep_running);

  launcher.launchThreads(detector);

  StatPublisher metrics(DetectorSettings.GraphiteAddress,
                        DetectorSettings.GraphitePort);

  Parser cmdParser(detector, mainStats, keep_running);
  Server cmdAPI(DetectorSettings.CommandServerPort, cmdParser);

  Timer LiveStats;

  while (true) {
    // Do not allow immediate exits
    if (RunTimer.timeUS() >= MicrosecondsPerSecond / 10) {
      if (keep_running == 0) {
        LOG(MAIN, Sev::Info, "Application stop, Exiting...");
        detector->stopThreads();
        sleep(1);
        break;
      }
    }

    if (RunTimer.timeUS() >=
        DetectorSettings.StopAfterSec * MicrosecondsPerSecond) {
      LOG(MAIN, Sev::Info, "Application timeout, Exiting...");
      detector->stopThreads();
      sleep(1);
      break;
    }

    if (LiveStats.timeUS() >= MicrosecondsPerSecond) {
      statUpTime = RunTimer.timeUS() / 1000000;
      metrics.publish(detector, mainStats);
      LiveStats.reset();
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
