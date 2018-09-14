/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class for handling calibration files for VMM asics
///
//===----------------------------------------------------------------------===//

#include <common/Log.h>
#include <common/Trace.h>
#include <gdgem/vmm3/CalibrationFile.h>
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB


/// \brief clear the calibration array
CalibrationFile::CalibrationFile() {
  auto n = sizeof(calibrations)/sizeof(calibration);
  for (size_t i = 0; i < n; i++) {
    ((calibration *)calibrations)[i] = nocorr;
  }
}


/// \brief load calibration from file
CalibrationFile::CalibrationFile(std::string jsonfile) : CalibrationFile() {
  std::ifstream t(jsonfile);
  std::string jsonstring((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());

  loadCalibration(jsonstring);
}


/// \brief parse json string with calibration data
void CalibrationFile::loadCalibration(std::string jsonstring) {
  nlohmann::json root;
  try {
    root = nlohmann::json::parse(jsonstring);
  }
  catch (...) {
    LOG(Sev::Warning, "Invalid Json file: {}", jsonstring);
    return;
  }

  auto vmmcal = root["vmm_calibration"];
  for (unsigned int i = 0; i < vmmcal.size(); i++) {
    auto fecid = vmmcal[i]["fecID"].get<unsigned int>();
    auto vmmid = vmmcal[i]["vmmID"].get<unsigned int>();
    auto offsets = vmmcal[i]["offsets"];
    auto slopes = vmmcal[i]["slopes"];
    XTRACE(INIT, DEB, "fecid: %d, vmmid: %d, offsets(%d), slopes(%d)\n", fecid, vmmid, offsets.size(), slopes.size());

    if ((slopes.size() != MAX_CH) or (offsets.size() != MAX_CH)) {
      LOG(Sev::Warning, "Invalid channel configuration, skipping for fec {} and vmm {}", fecid, vmmid);
      continue;
    }

    for (unsigned int j = 0; j < offsets.size(); j ++) {
      calibrations[fecid][vmmid][j].offset = offsets[j].get<float>();
      calibrations[fecid][vmmid][j].slope = slopes[j].get<float>();
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

CalibrationFile::calibration & CalibrationFile::getCalibration(unsigned int fecId, unsigned int vmmId, unsigned int chNo) {
  if ((fecId >= MAX_FEC) or (vmmId >= MAX_VMM) or (chNo >= MAX_CH)) {
    XTRACE(INIT, DEB, "invalid offsets: fec: %d, vmm: %d, ch:%d\n", fecId, vmmId, chNo);
    return errcorr;
  }

  return calibrations[fecId][vmmId][chNo];
}
