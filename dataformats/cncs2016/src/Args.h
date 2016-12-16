/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief command line argument parser
 */

#pragma once

#include <string>

class Args {
public:
  /** @brief constructor for program arguments parsed via getopt_long()
   * @param argc Argument count - typically taken from main()
   * @param argv Argument array - typically taken from main()
   */
  Args(int argc, char *argv[]);

  std::string dir{""};
  std::string prefix{""};
  std::string postfix{""};
  std::string ofile{"output"}; /**< output filename prefix */
  std::string cfile{"output"}; /**< calibration file */
  int start{0};                /**< start of file sequence number */
  int end{0};                  /**< end range of file sequence number */

  int runfile{0}; /**< if set, perform analysis from RawDataFiles.h */
  int sample_size{-1}; /**< number of events to generate histograms from */
  int hist_low{1};     /**< only include in heatmap/histogram values >= this */
};
