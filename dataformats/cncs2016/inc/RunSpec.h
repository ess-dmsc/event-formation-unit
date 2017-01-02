/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief definition of a data collection 'run'
 */

#pragma once
#include <string>

class RunSpec {
public:
  RunSpec(std::string dir, std::string prefix, std::string postfix,
      unsigned int start, unsigned int end, std::string ofile, std::string cfile)
        : dir_(dir)
        , prefix_(prefix)
        , postfix_(postfix)
        , start_(start)
        , end_(end)
        , ofile_(ofile)
        , cfile_(cfile) {};

  std::string dir_;
  std::string prefix_;
  std::string postfix_;
  unsigned int start_;
  unsigned int end_;
  std::string ofile_;
  std::string cfile_;
};
