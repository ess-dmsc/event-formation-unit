/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class for handling calibraiton files for VMM asics
///
//===----------------------------------------------------------------------===//

#include <common/Log.h>
#include <common/Trace.h>
#include <gdgem/vmm3/CalibrationFile.h>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

/// \brief clear the calibration array
CalibrationFile::CalibrationFile() {
  auto n = sizeof(calibrations)/sizeof(calibration_t);
  for (size_t i = 0; i < n; i++) {
    ((struct calibration_t *)calibrations)[i] = nocorr;
  }
}

void CalibrationFile::LoadCalibration(std::string jsonstring) {
    nlohmann::json root;
    try {
      root = nlohmann::json::parse(jsonstring);
    }
    catch (...) {
      LOG(Sev::Warning, "Invalid Json file: {}", jsonstring);
      return;
    }

    auto m = root["vmm_calibration"];
    for (unsigned int i = 0; i < m.size(); i++) {
      auto fecid = m[i]["fecID"].get<unsigned int>();
      auto vmmid = m[i]["vmmID"].get<unsigned int>();
      XTRACE(INIT, DEB, "fecid: %d, vmmid: %d\n", fecid, vmmid);
      auto offsets = m[i]["offsets"];
      auto slopes = m[i]["slopes"];
      if ((slopes.size() != MAX_CH) or (offsets.size() != MAX_CH)) {
        XTRACE(INIT, WAR, "Invalid calibration file (number of channels)\n");
        return;
      }
      for (unsigned int j = 0; j < offsets.size(); j ++) {
        auto offset = offsets[j].get<float>();
        auto slope = slopes[j].get<float>();
        calibrations[fecid][vmmid][j].offset = offset;
        calibrations[fecid][vmmid][j].slope = slope;
      }
    }
}

int CalibrationFile::addCalibration(unsigned int fecId, unsigned int vmmId, unsigned int chNo, float offset, float slope) {
  if ((fecId >= MAX_FEC) or (vmmId >= MAX_VMM) or (chNo >= MAX_CH)) {
    XTRACE(INIT, DEB, "invalid offsets: fec: %d, vmm: %d, ch:%d\n", fecId, vmmId, chNo);
    return -1;
  }
  calibrations[fecId][vmmId][chNo].slope = slope;
  calibrations[fecId][vmmId][chNo].offset = offset;
  return 0;
}

struct CalibrationFile::calibration_t & CalibrationFile::getCalibration(unsigned int fecId, unsigned int vmmId, unsigned int chNo) {
  if ((fecId >= MAX_FEC) or (vmmId >= MAX_VMM) or (chNo >= MAX_CH)) {
    XTRACE(INIT, DEB, "invalid offsets: fec: %d, vmm: %d, ch:%d\n", fecId, vmmId, chNo);
    return errcorr;
  }

  return calibrations[fecId][vmmId][chNo];
}
