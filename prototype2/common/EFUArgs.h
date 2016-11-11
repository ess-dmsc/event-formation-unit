/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Command line argument parser for EFU
 */

#pragma once
#include <common/Stats.h>
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

  std::string det{"nmx"};               /**< detector name */
  std::string broker{"localhost:9092"}; /**< Kafka broker */
  bool kafka{true};                     /**< whether to use Kafka or not */

  // NMX OPTIONS
  int reduction{80}; /**< data tuples in a cluster */

  // Runtime Stats
  Stats stat;
  unsigned int reportmask{0xffffffff};
};
