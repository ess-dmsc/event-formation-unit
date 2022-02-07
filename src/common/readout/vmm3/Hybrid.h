// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
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
  static constexpr int NumVMMs{2};          // #VMMs per cassette
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
  bool Initialised{false};
  std::string HybridId{""};
  uint8_t HybridNumber;
  std::vector<VMM3Calibration> VMMs{NumVMMs};
  uint16_t XOffset{0};
  uint16_t YOffset{0};
}; // class Hybrid

} // namespace ESSReadout
