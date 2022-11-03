/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Command line argument parser for EFU
///
//===----------------------------------------------------------------------===//

#pragma once

#include <CLI/CLI.hpp>
#include <common/detector/Detector.h>
#include <cstdint>
#include <string>

struct GraylogSettings {
  std::string address;
  std::uint16_t port;
};

class EFUArgs {
public:
  enum class Status { EXIT, CONTINUE };
  EFUArgs();
  Status parseFirstPass(const int argc, char *argv[]);

  Status parseSecondPass(const int argc, char *argv[]);

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
  bool parseLogLevel(std::vector<std::string> LogLevelString);
  int LogMessageLevel{6};
  std::string DetectorName;
  std::string LogFileName;

  bool PrintVersion{false};
  CLI::Option *DetectorOption;
  CLI::Option *HelpOption;

  GraylogSettings GraylogConfig{"127.0.0.1", 12201};
  BaseSettings EFUSettings;
};
