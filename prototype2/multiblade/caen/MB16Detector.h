/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// /brief Multiblade geometry
///
//===----------------------------------------------------------------------===//

#pragma once
#include <cstdio>
#include <vector>

namespace Multiblade {

class MB16Detector {
public:

  struct Digitiser {
    int index; // order in which they are positioned in VME crate
    int digid; // digitiser id
  };

  MB16Detector(std::vector<struct MB16Detector::Digitiser> & digitisers) : Digitisers(digitisers) { };

  /// \brief mapping between digitiser ids (serial numbers) and physical order
  inline int cassette(int digid) {
    for (auto & digi : Digitisers) {
      if (digi.digid == digid) {
        return digi.index;
      }
    }
    return -1;
  }
  
private:
  std::vector<struct MB16Detector::Digitiser> & Digitisers;
};

}
