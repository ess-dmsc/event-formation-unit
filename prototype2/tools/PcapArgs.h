/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Command line argument parser for udpgen_pcap
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <string>

class PcapArgs {
public:
  /// \brief constructor for program arguments parsed via getopt_long()
  /// \param argc Argument count - typically taken from main()
  /// \param argv Argument array - typically taken from main()
  PcapArgs(int argc, char *argv[]);

  std::string filename{}; ///< for single file streaming
  uint64_t txPkt{0xffffffffffffffff};
  std::string dest_ip{"127.0.0.1"}; ///< destination ip address
  int port{9000};                   ///< destination udp port
  int throttle{0};   ///< actually a sleep() counter
  int loop{0};       ///< single shot or loop
  int pcapoffset{0}; ///< for pcap: start after offset
};
// GCOVR_EXCL_STOP
