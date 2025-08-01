// Copyright (C) 2016 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Command line argument parser for EFU
///
//===----------------------------------------------------------------------===//

#pragma once

#include <CLI/CLI.hpp>
#include <common/detector/BaseSettings.h>
#include <cstdint>
#include <string>

struct GraylogSettings {
  std::string address;
  uint16_t port;
};

class EFUArgs {
public:
  enum class Status { EXIT, ERREXIT, CONTINUE };
  EFUArgs();
  Status parseArgs(const int argc, char *argv[]);

  void printHelp();

  void printVersion();

  void printSettings();

  std::string getDetectorName() { return DetectorName; };
  GraylogSettings getGraylogSettings() { return GraylogConfig; };

  int getLogLevel() { return LogMessageLevel; };
  std::string getLogFileName() { return LogFileName; };

  BaseSettings getBaseSettings() { return EFUSettings; };

  CLI::App CLIParser{"Event formation unit (efu)"};

  int buflen{9000}; ///< rx buffer length (B)

private:
  bool parseLogLevel(const std::vector<std::string> &LogLevelString);
  int LogMessageLevel{6};
  std::string DetectorName;
  std::string LogFileName;

  bool PrintVersion{false};
  CLI::Option *HelpOption;

  GraylogSettings GraylogConfig{"127.0.0.1", 12201};
  BaseSettings EFUSettings;
};
