/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Command line argument parser for EFU
 */

#pragma once
#include <CLI/CLI11.hpp>
#include <common/Detector.h>
#include <cstdint>
#include <multigrid/mgcncs/ChanConv.h>
#include <string>

struct GraylogSettings {
  std::string address;
  std::uint16_t port;
};

struct ThreadCoreAffinitySetting {
  std::string Name;
  std::uint16_t Core;
};

class EFUArgs {
public:
  EFUArgs();
  bool parseAndProceed(const int argc, char *argv[]);

  void printHelp();

  void printSettings();

  std::string getDetectorName() { return det; };
  GraylogSettings getGraylogSettings() { return GraylogConfig; };

  std::vector<ThreadCoreAffinitySetting> getThreadCoreAffinity() {
    return ThreadAffinity;
  };

  bool parseAgain(const int argc, char *argv[]);

  BaseSettings GetBaseSettings() { return EFUSettings; };

  CLI::App CLIParser{"Event formation unit (efu)"};

  int buflen{9000}; /**< rx buffer length (B) */

  std::string det; /**< detector name */

  // Runtime Stats
  // EFUStats stat;

  // Pipeline-specific configuration
private:
  bool parseAffinityStrings(std::vector<std::string> ThreadAffinityStrings);

  std::vector<ThreadCoreAffinitySetting> ThreadAffinity;
  CLI::Option *detectorOption;

  GraylogSettings GraylogConfig{"127.0.0.1", 12201};
  BaseSettings EFUSettings;
};
