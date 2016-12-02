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

  std::string dir{"/home/morten/cncsdata/vanadium_july_27/"};
  std::string prefix{"2016_07_26_1005_sample_"};
  std::string postfix{".bin"};
  std::string ofile{"output"};
  int start{0};
  int end{0};
};
