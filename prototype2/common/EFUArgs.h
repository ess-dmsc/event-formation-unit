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

class EFUArgs {
public:
  EFUArgs();
  bool parseAndProceed(const int argc, char *argv[]);
  
  void printHelp();
  
  void printSettings();
  
  std::string getDetectorName() {return det;};
  GraylogSettings getGraylogSettings() {return GraylogConfig;};
  
  bool parseAgain(const int argc, char *argv[]);
  
  StdSettings GetDefaultSettings() {return EFUSettings;};
  
  CLI::App CLIParser{"Event formation unit (efu)"};
  
//  enum thread_cmd { NOCMD =0, EXIT, THREAD_LOADCAL, THREAD_TERMINATE};

  int cpustart{12}; /**< lcore id for input processing thread */

//  std::string ip_addr{"0.0.0.0"}; /**< used for data generators */
  int buflen{9000};               /**< rx buffer length (B) */

  unsigned int updint{1};             /**< update interval (s) */
  unsigned int stopafter{0xffffffff}; /**< 'never' stop */

  std::string det;             /**< detector name */
//  std::string broker{"localhost:9092"}; /**< Kafka broker */
  bool kafka{true};                     /**< whether to use Kafka or not */

  std::string graphite_ip_addr{"127.0.0.1"}; /**< graphite time series db */
  int graphite_port{2003};                   /**< graphite time series db */

  int cmdserver_port{8888}; /**< for command line API */

  // Runtime Stats
  // EFUStats stat;
  unsigned int reportmask{0x2};

  // Pipeline-specific configuration
  std::string config_file;

  // IPC data for communicating between main and threads
//  uint16_t wirecal[CSPECChanConv::adcsize];
//  uint16_t gridcal[CSPECChanConv::adcsize];
//  thread_cmd proc_cmd{NOCMD};
private:
  CLI::Option *detectorOption;
  
  GraylogSettings GraylogConfig{"127.0.0.1", 12201};
  StdSettings EFUSettings{"0.0.0.0", 9000, 2000000, 2000000, "localhost", 9092, "Detector_data"};
};
