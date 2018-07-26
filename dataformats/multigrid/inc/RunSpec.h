/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief definition of a data collection 'run'
///
//===----------------------------------------------------------------------===//

#pragma once
#include <string>

class RunSpec {
public:
  RunSpec(std::string dir, std::string prefix, std::string postfix,
          unsigned int start, unsigned int end, std::string ofile,
          unsigned int thresh)
      : dir_(dir), prefix_(prefix), postfix_(postfix), start_(start), end_(end),
        ofile_(ofile), thresh_(thresh){};

  std::string dir_;
  std::string prefix_;
  std::string postfix_;
  unsigned int start_;
  unsigned int end_;
  std::string ofile_;
  unsigned int thresh_;
};
