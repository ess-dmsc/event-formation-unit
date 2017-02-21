/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Command line argument parser for EFU
 */

#pragma once
#include <common/Detector.h>
#include <cspec/CSPECChanConv.h>
#include <string>

class EFUArgs {
public:
  /** @brief constructor for program arguments parsed via getopt_long()
   * @param argc Argument count - typically taken from main()
   * @param argv Argument array - typically taken from main()
   */
  EFUArgs(int argc, char *argv[]);

  int cpustart{12}; /**< lcore id for input processing thread */

  std::string ip_addr{"0.0.0.0"}; /**< used for data generators */
  int port{9000};                 /**< udp receive port */
  int buflen{9000};               /**< rx buffer length (B) */
  int rcvbuf{1000000};            /**< socket rx buffer size (rmem_max) */
  int sndbuf{1000000};            /**< soxket tx buffer size (wmem_max) */

  unsigned int updint{1};             /**< update interval (s) */
  unsigned int stopafter{0xffffffff}; /**< 'never' stop */

  std::string det{"cspec"};             /**< detector name */
  std::string broker{"localhost:9092"}; /**< Kafka broker */
  bool kafka{true};                     /**< whether to use Kafka or not */

  std::string graphite_ip_addr{"127.0.0.1"}; /**< graphite time series db */
  int graphite_port{2003};                   /**< graphite time series db */

  int cmdserver_port{8888}; /**< for command line API */

  // NMX OPTIONS
  int reduction{80}; /**< data tuples in a cluster */

  // Runtime Stats
  // EFUStats stat;
  unsigned int reportmask{0x2};

  // IPC data for communicating between main and threads
  uint16_t wirecal[CSPECChanConv::adcsize];
  uint16_t gridcal[CSPECChanConv::adcsize];
  uint32_t proc_cmd{0};

  std::shared_ptr<Detector> detectorif; /**< @todo is this the place? */
};

// Used all the time and is not global variable
extern EFUArgs *efu_args;
