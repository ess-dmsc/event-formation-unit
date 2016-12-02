/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class doing batch reading, event formation, stats and output
 */
#pragma once

#include <Histogram.h>
#include <cspec/CSPECData.h>
#include <string>

class Analyze {
public:
  Analyze(std::string ofile_prefix);
  ~Analyze();
  int batchreader(std::string dir, std::string fileprefix,
                  std::string filepostfix, int begin, int end);

private:
  int readfile(std::string filename);
  int populate(CSPECData &dat, int readouts);
  int eventdatafd{-1};
  int histdatafd{-1};
  Histogram global;
  Histogram local;
  std::string ofile{"data"};
  struct {
    unsigned int readouts;
    unsigned int discards;
    unsigned int events;
  } stats;
};
