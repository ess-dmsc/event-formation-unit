/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Command line argument parser for cspecgen
///
//===----------------------------------------------------------------------===//
/// GCOVR_EXCL_START

#pragma once

#include <string>

class DGArgs {
public:
  /// \brief constructor for program arguments parsed via getopt_long()
  /// \param argc Argument count - typically taken from main()
  /// \param argv Argument array - typically taken from main()
  DGArgs(int argc, char *argv[]);

  std::string filename{}; ///< for single file streaming

  unsigned int txGB{10}; ///< total transmit size (GB)
  uint64_t txPkt{0xffffffffffffffff};
  int txEvt{100}; ///< 100 events per packet

  std::string dest_ip{"127.0.0.1"}; ///< destination ip address
  int port{9000};                   ///< destination udp port
  int buflen{9000};                 ///< Tx buffer size
  int sndbuf{1000000};              ///< kernel sndbuf size

  int speed_level{0}; /// for arbitrary use in sleep() or usleep()
  int repeat{1};      /// repetition count for arbitrary use

  unsigned int updint{1}; ///< update interval (seconds)
};
/// GCOVR_EXCL_STOP
