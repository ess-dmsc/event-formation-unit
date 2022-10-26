// Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief EFU main application
///
//===----------------------------------------------------------------------===//

#include <boost/filesystem.hpp>
#include <common/StatPublisher.h>
#include <common/Version.h>
#include <common/debug/Log.h>
#include <common/detector/EFUArgs.h>
#include <common/system/gccintel.h>
#include <common/time/Timer.h>
#include <cstdlib>
#include <efu/ExitHandler.h>
#include <efu/HwCheck.h>
#include <efu/Launcher.h>
#include <efu/Loader.h>
#include <efu/Parser.h>
#include <efu/Server.h>
#include <iostream>
#include <unistd.h> // sleep()
#include <vector>

static constexpr uint64_t MicrosecondsPerSecond{1000000};

std::string ConsoleFormatter(const Log::LogMessage &Msg) {
  static const std::vector<std::string> SevToString{
      "EMG", "ALR", "CRIT", "ERR", "WAR", "NOTE", "INFO", "DEB"};
  std::string FileName;
  std::int64_t LineNr = -1;
  for (auto &CField : Msg.AdditionalFields) {
    if (CField.first == "file") {
      FileName = CField.second.strVal;
    } else if (CField.first == "line") {
      LineNr = CField.second.intVal;
    }
  }
  return fmt::format("{:5}{:21}{:5} - {}",
                     SevToString.at(static_cast<size_t>(Msg.SeverityLevel)),
                     FileName, LineNr, Msg.MessageString);
}

std::string FileFormatter(const Log::LogMessage &Msg) {
  std::time_t cTime = std::chrono::system_clock::to_time_t(Msg.Timestamp);
  char timeBuffer[50];
  size_t bytes = std::strftime(timeBuffer, 50, "%F %T", std::localtime(&cTime));
  static const std::vector<std::string> SevToString{
      "EMG", "ALR", "CRIT", "ERR", "WAR", "NOTE", "INFO", "DEB"};
  std::string FileName;
  std::int64_t LineNr = -1;
  for (auto &CField : Msg.AdditionalFields) {
    if (CField.first == "file") {
      FileName = CField.second.strVal;
    } else if (CField.first == "line") {
      LineNr = CField.second.intVal;
    }
  }
  return fmt::format("{} {:5}{:21}:{:<5} - {}", std::string(timeBuffer, bytes),
                     SevToString.at(static_cast<size_t>(Msg.SeverityLevel)),
                     FileName, LineNr, Msg.MessageString);
}

void EmptyGraylogMessageQueue() {
  std::vector<Log::LogHandler_P> GraylogHandlers(Log::GetHandlers());
  if (not GraylogHandlers.empty()) {
    int WaitLoops = 25;
    auto SleepTime = std::chrono::milliseconds(20);
    bool ContinueLoop = true;
    for (int i = 0; i < WaitLoops and ContinueLoop; i++) {
      std::this_thread::sleep_for(SleepTime);
      ContinueLoop = false;
      for (auto &Ptr : GraylogHandlers) {
        if (not Ptr->emptyQueue()) {
          ContinueLoop = true;
        }
      }
    }
  }
  Log::RemoveAllHandlers();
}

/** Load detector, launch pipeline threads, then sleep until timeout or break */
int main(int argc, char *argv[]) {
  BaseSettings DetectorSettings;
  std::shared_ptr<Detector> detector;
  std::string DetectorName;
  GraylogSettings GLConfig;
  Loader loader;
  HwCheck hwcheck;
  Timer RunTimer;

  { // Make sure that the EFUArgs instance is deallocated before the detector
    // plugin is
    EFUArgs efu_args;
    if (EFUArgs::Status::EXIT == efu_args.parseFirstPass(argc, argv)) {
      return 0;
    }
    // Set-up logging before we start doing important stuff
    Log::RemoveAllHandlers();

    auto CI = new Log::ConsoleInterface();
    CI->setMessageStringCreatorFunction(ConsoleFormatter);
    Log::AddLogHandler(CI);

    Log::SetMinimumSeverity(Log::Severity(efu_args.getLogLevel()));
    if (!efu_args.getLogFileName().empty()) {
      auto FI = new Log::FileInterface(efu_args.getLogFileName());
      FI->setMessageStringCreatorFunction(FileFormatter);
      Log::AddLogHandler(FI);
    }

    loader.loadPlugin(efu_args.getDetectorName());
    if (not loader.IsOk()) {
      efu_args.printHelp();
      EmptyGraylogMessageQueue();
      return -1;
    }

    GLConfig = efu_args.getGraylogSettings();
    if (not GLConfig.address.empty()) {
      Log::AddLogHandler(
          new Log::GraylogInterface(GLConfig.address, GLConfig.port));
    }
    efu_args.printSettings();
    DetectorSettings = efu_args.getBaseSettings();

    /// Automatically create the Graphite metric prefix from detector plugin
    /// name remove leading path and trailing extension
    auto p = boost::filesystem::path(efu_args.getDetectorName())
                 .filename()
                 .stem()
                 .string();
    DetectorSettings.GraphitePrefix = std::string("efu.") + p;

    detector = loader.createDetector(DetectorSettings);
    DetectorName = efu_args.getDetectorName();
  }

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
      detector.reset(); // De-allocate detector before we unload detector module
      EmptyGraylogMessageQueue();
      return -1;
    }

    // if (hwcheck.checkDiskSpace(hwcheck.DirectoriesToCheck) == false) {
    //   LOG(MAIN, Sev::Error, "Not enough space on filesystem");
    //   LOG(MAIN, Sev::Error, "exiting...");
    //   detector.reset(); //De-allocate detector before we unload detector
    //   module EmptyGraylogMessageQueue(); return -1;
    // }
  }

  if (DetectorSettings.StopAfterSec == 0) {
    LOG(MAIN, Sev::Info, "Event Formation Unit Exit (Immediate)");
    detector.reset(); // De-allocate detector before we unload detector module
    EmptyGraylogMessageQueue();
    return 0;
  }

  LOG(MAIN, Sev::Info, "Launching EFU as Instrument {}", DetectorName);

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

    if ((livestats.timeus() >= MicrosecondsPerSecond) && detector != nullptr) {
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

  detector.reset();

  EmptyGraylogMessageQueue();
  return 0;
}
