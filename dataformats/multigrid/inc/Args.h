/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief command line argument parser
///
//===----------------------------------------------------------------------===//

#pragma once

#include <string>

class Args {
public:
  /// \brief constructor for program arguments parsed via getopt_long()
  /// \param argc Argument count - typically taken from main()
  /// \param argv Argument array - typically taken from main()
  // clang-format off
  Args(int argc, char *argv[]);

  std::string basedir{""}; ///< To support different locations for files
  std::string dir{""};       ///< location of file relative to basedir
  std::string prefix{""};    ///< filename prefix
  std::string postfix{""};   ///< filename postfix
  std::string ofile{"output"}; ///< output filename prefix
  int start{0};              ///< start of file sequence number
  int end{0};                ///< end range of file sequence number

  std::string runfile{""};   ///< if set, perform analysis from json filelist
  std::string runspec{"validruns"};
  int sample_size{-1};       ///< # of events to generate histograms from
  int hist_low{1};           ///< only include in heatmap/hist. values >= this
  // clang-format on
};
