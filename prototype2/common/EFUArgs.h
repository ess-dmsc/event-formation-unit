/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Command line argument parser for EFU
 */

#pragma once
#include <common/Detector.h>
#include <multigrid/mgcncs/ChanConv.h>
#include <string>
#include <CLI/CLI11.hpp>
#include <cstdint>

struct GraylogSettings {
  std::string address;
  std::uint16_t port;
};

struct ThreadCoreAffinity {
  std::string Name;
  std::uint16_t Core;
};

class EFUArgs {
public:
  EFUArgs();
  bool parseAndProceed(const int argc, char *argv[]);

  void printHelp();

  void printSettings();

  std::string getDetectorName() {return det;};
  GraylogSettings getGraylogSettings() {return GraylogConfig;};

  std::vector<ThreadCoreAffinity> getThreadCoreAffinity() {return ThreadAffinity;};

  bool parseAgain(const int argc, char *argv[]);
  
  BaseSettings GetBaseSettings() {return EFUSettings;};

  CLI::App CLIParser{"Event formation unit (efu)"};

  int buflen{9000};               /**< rx buffer length (B) */

  unsigned int stopafter{0xffffffff}; /**< 'never' stop */

  std::string det;             /**< detector name */

  std::string graphite_ip_addr{"127.0.0.1"}; /**< graphite time series db */
  int graphite_port{2003};                   /**< graphite time series db */

  int cmdserver_port{8888}; /**< for command line API */

  // Runtime Stats
  // EFUStats stat;

  // Pipeline-specific configuration

  // IPC data for communicating between main and threads
//  uint16_t wirecal[CSPECChanConv::adcsize];
//  uint16_t gridcal[CSPECChanConv::adcsize];
private:
  bool parseAffinityStrings(std::vector<std::string> ThreadAffinityStrings);

  std::vector<ThreadCoreAffinity> ThreadAffinity;
  CLI::Option *detectorOption;

  GraylogSettings GraylogConfig{"127.0.0.1", 12201};
  BaseSettings EFUSettings{"0.0.0.0", 9000, 2000000, 2000000, "localhost", 9092, "Detector_data", 1};
};
