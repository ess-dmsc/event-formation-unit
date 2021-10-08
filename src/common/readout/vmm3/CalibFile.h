// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Get Hybrid/VMM3 calibration data from json
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/JsonFile.h>
#include <common/debug/Trace.h>
#include <common/readout/vmm3/Hybrid.h>
#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace ESSReadout {

class CalibFile {
public:

  CalibFile(std::string DetectorName, std::vector<Hybrid> & Hybrids)
     : Detector(DetectorName), Hybrids(Hybrids) {};


  void load(std::string FileName);

  void apply();

  void applyCalibration(unsigned HybridIndex, nlohmann::json Calibration);
  void applyVMM3Calibration(Hybrid & Hybrid, unsigned vmmid,
    nlohmann::json VMMCalibration);

  std::string Detector{""};
  std::vector<Hybrid> & Hybrids;
  static constexpr unsigned VMM0{0};
  static constexpr unsigned VMM1{1};

  nlohmann::json root;
};

} // namespace ESSReadout
