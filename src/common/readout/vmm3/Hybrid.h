// Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Contains VMM3 Hybrid related settings
///
/// Each Hybrid has two VMMs, hence the vector of 2 VMM3Calibrations
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <common/readout/vmm3/VMM3Calibration.h>
#include <string>
#include <vector>

namespace ESSReadout {

class Hybrid {
public:
  static constexpr int NumVMMs{2};          // #VMMs per hybrid
  static constexpr unsigned int IdSize{32}; // chars

  static bool isAvailable(std::string NewId, std::vector<Hybrid> &Hybrids) {
    for (unsigned i = 0; i < Hybrids.size(); i++) {
      if (Hybrids[i].HybridId == NewId) {
        XTRACE(INIT, ALW, "Id '%s' is already used in Hybrid %d", NewId.c_str(),
               i);
        return false;
      }
    }
    return true;
  }

  // indicates if hybrid was initialised in config file
  bool Initialised{false};
  
  std::string HybridId{""};
  uint8_t HybridNumber;
  
  // holds calibration info for each VMM ASIC on hybrid
  std::vector<VMM3Calibration> VMMs{NumVMMs};

  // defines offset of hybrid in logical geometry
  uint16_t XOffset{0};
  uint16_t YOffset{0};

  //defines the minimum ADC value threshold, under this is discarded as noise
  uint16_t MinADC{0};
}; // class Hybrid

} // namespace ESSReadout
