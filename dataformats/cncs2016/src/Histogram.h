/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief simple histogram class
 */

#pragma once

#include <cassert>
#include <string.h>
#include <vector>

class Histogram {
public:
  static const unsigned int histsize = 16384;
  std::vector<int> hist;
  int entries{0};
  int firstnonzero = -1;
  int lastnonzero = -1;
  int nonzero = 0;

  Histogram() { /**< @todo parametrize with size */
    hist.reserve(histsize);
    clear();
  }

  void add(unsigned int value) {
    assert(value < histsize);
    hist[value]++;
    entries++;
  }

  void clear() {
    for (unsigned int i = 0; i < Histogram::histsize; i++) {
      hist[i] = 0;
    }
    entries = 0;
  }

  void analyze(int low_cut) {
    firstnonzero = -1;
    lastnonzero = -1;
    nonzero = 0;

    for (unsigned int i = low_cut; i < histsize; i++) {
      if ((hist[i] > 0) and (firstnonzero == -1)) {
        firstnonzero = i;
      }
      if (hist[i] > 0) {
        nonzero++;
        lastnonzero = i;
      }
    }
  }
};
