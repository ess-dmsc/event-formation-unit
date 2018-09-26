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
#include <fstream>
#include <gdgem/vmm3/CalibrationFile.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

/// \brief clear the calibration array
CalibrationFile::CalibrationFile() {
  constexpr size_t NumberEntries = (size_t)(sizeof(Calibrations) / sizeof(Calibration));

  static_assert(sizeof(Calibration) == 8, "struct packing issue");
  static_assert(NumberEntries == MAX_FEC * MAX_VMM * MAX_CH, "calibration table size mismatch");

  for (size_t i = 0; i < NumberEntries; i++) {
    ((Calibration *)Calibrations)[i] = NoCorr;
  }
}

/// \brief load calibration from file
CalibrationFile::CalibrationFile(std::string jsonfile) : CalibrationFile() {

  if (jsonfile.empty()) {
    return;
  }

  LOG(INIT, Sev::Info, "Loading calibration file {}", jsonfile);

  std::ifstream t(jsonfile);
  std::string Jsonstring((std::istreambuf_iterator<char>(t)),
                         std::istreambuf_iterator<char>());

  loadCalibration(Jsonstring);
}

/// \brief parse json string with calibration data
void CalibrationFile::loadCalibration(std::string jsonstring) {
  nlohmann::json Root;
  try {
    Root = nlohmann::json::parse(jsonstring);
  } catch (...) {
    LOG(INIT, Sev::Warning, "Invalid Json file: {}", jsonstring);
    return;
  }

  try {
    auto VmmCals = Root["vmm_calibration"];
    for (auto &vmmcal : VmmCals) {
      auto fecid = vmmcal["fecID"].get<unsigned int>();
      auto vmmid = vmmcal["vmmID"].get<unsigned int>();
      auto offsets = vmmcal["offsets"];
      auto slopes = vmmcal["slopes"];

      XTRACE(INIT, DEB, "fecid: %d, vmmid: %d, offsets(%d), slopes(%d)\n", fecid,
             vmmid, offsets.size(), slopes.size());

      if ((slopes.size() != MAX_CH) or (offsets.size() != MAX_CH)) {
        LOG(INIT, Sev::Warning,
            "Invalid channel configuration, skipping for fec {} and vmm {}",
            fecid, vmmid);
        continue;
      }

      for (unsigned int j = 0; j < offsets.size(); j++) {
        Calibrations[fecid][vmmid][j].offset = offsets[j].get<float>();
        Calibrations[fecid][vmmid][j].slope = slopes[j].get<float>();
      }
    }
  }
  catch (const std::exception &exc) {
    LOG(INIT, Sev::Error, "JSON config - invalid json: {}", exc.what());
  }
}

bool CalibrationFile::addCalibration(unsigned int fecId, unsigned int vmmId,
                                     unsigned int chNo, float offset,
                                     float slope) {
  if ((fecId >= MAX_FEC) or (vmmId >= MAX_VMM) or (chNo >= MAX_CH)) {
    XTRACE(INIT, DEB, "invalid offsets: fec: %d, vmm: %d, ch:%d\n", fecId,
           vmmId, chNo);
    return false;
  }
  Calibrations[fecId][vmmId][chNo].slope = slope;
  Calibrations[fecId][vmmId][chNo].offset = offset;
  return true;
}

CalibrationFile::Calibration &
CalibrationFile::getCalibration(unsigned int fecId, unsigned int vmmId,
                                unsigned int chNo) {
  if ((fecId >= MAX_FEC) or (vmmId >= MAX_VMM) or (chNo >= MAX_CH)) {
    XTRACE(INIT, DEB, "invalid offsets: fec: %d, vmm: %d, ch:%d\n", fecId,
           vmmId, chNo);
    return ErrCorr;
  }

  return Calibrations[fecId][vmmId][chNo];
}
