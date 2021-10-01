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

#include <common/readout/vmm3/VMM3Calibration.h>
#include <vector>

namespace ESSReadout {

class Hybrid {
public:
  static constexpr int NumVMMs{2}; // #VMMs per cassette

  std::vector<VMM3Calibration> VMMs{NumVMMs};
}; // class Hybrid

} // namespace ESSReadout
