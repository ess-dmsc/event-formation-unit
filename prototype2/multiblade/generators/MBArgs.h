//
// Copied from NMXArgs.h
//
/// GCOVR_EXCL_START
#include <string>

#pragma once

class MBArgs {
public:
  /** @brief constructor for program arguments parsed via getopt_long()
   * @param argc Argument count - typically taken from main()
   * @param argv Argument array - typically taken from main()
   */
  MBArgs(int argc, char *argv[]);

  std::string filename{}; /**< for single file streaming */
  std::string outfile{};  /**< for single file streaming */

  unsigned int txGB{10}; /**< total transmit size (GB) */
  uint64_t txPkt{0xffffffffffffffff};
  uint64_t dppkg{0xffffffffffffffff}; /** Max */

  std::string dest_ip{"127.0.0.1"}; /**< destination ip address */
  uint16_t port{9000};              /**< destination udp port */
  uint16_t buflen{9000};            /**< Tx buffer size */
  unsigned int sndbuf{1000000};     /**< kernel sndbuf size */

  int speed_level{0};

  unsigned int updint{1}; /**< update interval (seconds) */
};
/// GCOVR_EXCL_STOP
