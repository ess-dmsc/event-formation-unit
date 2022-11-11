// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Collection of Graylog methods for EFU
///
//===----------------------------------------------------------------------===//

#include <efu/Graylog.h>

std::string Graylog::ConsoleFormatter(const Log::LogMessage &Msg) {
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

std::string Graylog::FileFormatter(const Log::LogMessage &Msg) {
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

void Graylog::EmptyGraylogMessageQueue() {
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

void Graylog::AddLoghandlerForNetwork(std::string DetectorName,
                                      std::string FileName, int LogLevel,
                                      std::string Address, int Port) {
  // Set-up logging before we start doing important stuff
  // change process to "efu" and set process_name to Instrument
  std::string ProcessKey{"process"};
  std::string ProcessValue{"efu"};
  std::string ProcessNameKey{"process_name"};
  Log::RemoveAllHandlers();
  Log::Logger::Inst().addField(ProcessKey, ProcessValue);
  Log::Logger::Inst().addField(ProcessNameKey, DetectorName);

  auto CI = new Log::ConsoleInterface();
  CI->setMessageStringCreatorFunction(ConsoleFormatter);
  Log::AddLogHandler(CI);

  Log::SetMinimumSeverity(Log::Severity(LogLevel));
  if (!FileName.empty()) {
    auto FI = new Log::FileInterface(FileName);
    FI->setMessageStringCreatorFunction(FileFormatter);
    Log::AddLogHandler(FI);
  }

  if (not Address.empty()) {
    Log::AddLogHandler(new Log::GraylogInterface(Address, Port));
  }
}
