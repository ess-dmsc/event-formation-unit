/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/EFUArgs.h>
#include <common/StatPublisher.h>
#include <common/Log.h>
#include <common/Version.h>
#include <efu/ExitHandler.h>
#include <efu/HwCheck.h>
#include <efu/Launcher.h>
#include <efu/Loader.h>
#include <efu/Parser.h>
#include <efu/Server.h>
#include <libs/include/Timer.h>
#include <libs/include/gccintel.h>
#include <unistd.h> // sleep()
#include <vector>

#define ONE_SECOND_US 1000000U

std::string ConsoleFormatter(const Log::LogMessage &Msg) {
  static const std::vector<std::string> SevToString{"EMG", "ALR", "CRI", "ERR", "WAR", "NOTE", "INF", "DEB"};
  std::string FileName;
  std::int64_t LineNr = -1;
  for (auto &CField : Msg.AdditionalFields) {
    if (CField.first == "file") {
      FileName = CField.second.strVal;
    } else if (CField.first == "line") {
      LineNr = CField.second.intVal;
    }
  }
  return fmt::format("{:5}{:21}{:5} - {}", SevToString.at(int(Msg.SeverityLevel)), FileName, LineNr, Msg.MessageString);
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
    // Set-up logging before we start doing important stuff
    Log::RemoveAllHandlers();

    auto CI = new Log::ConsoleInterface();
    CI->setMessageStringCreatorFunction(ConsoleFormatter);
    Log::AddLogHandler(CI);

    Log::SetMinimumSeverity(Log::Severity(efu_args.getLogLevel()));
    if (efu_args.getLogFileName().size() > 0) {
      Log::AddLogHandler(new Log::FileInterface(efu_args.getLogFileName()));
    }

    loader.loadPlugin(efu_args.getDetectorName());
    if (not loader.IsOk()) {
      efu_args.printHelp();
      EmptyGraylogMessageQueue();
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
    GLConfig = efu_args.getGraylogSettings();
    if (not GLConfig.address.empty()) {
      Log::AddLogHandler(new Log::GraylogInterface(GLConfig.address, GLConfig.port));
    }
    efu_args.printSettings();
    DetectorSettings = efu_args.getBaseSettings();
    detector = loader.createDetector(DetectorSettings);
    AffinitySettings = efu_args.getThreadCoreAffinity();
    DetectorName = efu_args.getDetectorName();
  }

  hwcheck.setMinimumMTU(DetectorSettings.MinimumMTU);

  int keep_running = 1;

  ExitHandler::InitExitHandler();

  LOG(MAIN, Sev::Info, "Starting Event Formation Unit");
  LOG(MAIN, Sev::Info, "Event Formation Unit version: {}", efu_version());
  LOG(MAIN, Sev::Info, "Event Formation Unit build: {}", efu_buildstr());

  if (hwcheck.checkMTU(hwcheck.defaultIgnoredInterfaces) == false) {
    LOG(MAIN, Sev::Error, "MTU checks failed, for a quick fix, try");
    LOG(MAIN, Sev::Error, "sudo ifconfig eth0 mtu 9000 (change eth0 to match your system)");
    LOG(MAIN, Sev::Error, "exiting...");
    detector.reset(); //De-allocate detector before we unload detector module
    EmptyGraylogMessageQueue();
    return -1;
  }

  if (DetectorSettings.StopAfterSec == 0) {
    LOG(MAIN, Sev::Info, "Event Formation Unit Exit (Immediate)");
    detector.reset(); //De-allocate detector before we unload detector module
    EmptyGraylogMessageQueue();
    return 0;
  }


  LOG(MAIN, Sev::Info, "Launching EFU as Instrument {}", DetectorName);

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
        LOG(MAIN, Sev::Info, "Application stop, Exiting...");
        detector->stopThreads();
        sleep(1);
        break;
      }
    }

    if (RunTimer.timeus() >=
        DetectorSettings.StopAfterSec * (uint64_t)ONE_SECOND_US) {
      LOG(MAIN, Sev::Info, "Application timeout, Exiting...");
      detector->stopThreads();
      sleep(1);
      break;
    }

    if (livestats.timeus() >= ONE_SECOND_US && detector != nullptr) {
      metrics.publish(detector);
      livestats.now();
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
