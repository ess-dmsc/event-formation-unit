/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Command line argument parser for cspecgen
 */
/// GCOVR_EXCL_START
#pragma once
#include <string>

class JsonArgs {
public:
  /** @brief constructor for program arguments parsed via getopt_long()
   * @param argc Argument count - typically taken from main()
   * @param argv Argument array - typically taken from main()
   */
  JsonArgs(int argc, char *argv[]);

  std::string basedir{};          /**< basedir for all files */
  std::string runfile{};          /**< read data from file */
  std::string runspecification{}; /**< use this run definition */

  std::string dest_ip{"127.0.0.1"}; /**< destination ip address */
  int port{9000};                   /**< destination udp port */
  int buflen{9000};                 /**< Tx buffer size */
  int sndbuf{1000000};              /**< kernel sndbuf size */

  int speed_level{0};

  unsigned int updint{1}; /**< update interval (seconds) */
};
/// GCOVR_EXCL_STOP
