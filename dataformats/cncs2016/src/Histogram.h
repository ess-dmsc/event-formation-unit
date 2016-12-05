/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief simple histogram class
 */

#pragma once

#include <cassert>
#include <string.h>

class Histogram {
public:
  static const unsigned int histsize = 16384;
  int hist[histsize];
  int entries{0};
  int firstnonzero = -1;
  int lastnonzero = -1;
  int nonzero = 0;

  Histogram() { clear(); }

  void add(unsigned int value) {
    assert(value < histsize);
    hist[value]++;
    entries++;
  }

  void clear() {
    memset(hist, 0, sizeof(hist));
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
