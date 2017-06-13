/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class doing batch reading, event formation, stats and output
 */
#pragma once

#include <Args.h>
#include <Histogram.h>
#include <multigrid/mgcncs/Data.h>
#include <string>

class Analyze {
public:
  Analyze(Args &opts);
  ~Analyze();
  int batchreader(std::string dir, std::string fileprefix,
                  std::string filepostfix, int begin, int end);
  void makecal(unsigned int thresh);

private:
  int readfile(std::string filename);
  int populate(CSPECData &dat, int readouts);

  Histogram global, local; /**< for file output */
  Histogram w0pos, g0pos;  /**< for use with PeakFinder */

  std::string ofile{};

  struct {
    unsigned int readouts;
    unsigned int discards;
    unsigned int events;
  } stats;

  int seqno{0};        /**< used to enumerate events */
  int low_cut{1};      /**< used when writing histogram data to file */
  int eventdatafd{-1}; /**< file descriptor for event data file */
  int histdatafd{-1};  /**< file descriptor for histogram data file */
  int csvdatafd{-1};   /**< file descriptor for csv data file */
};
