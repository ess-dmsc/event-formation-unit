// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read calibrations from file
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <modules/caen/geometry/CDCalibration.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

/// loads calibration from file
CDCalibration::CDCalibration(std::string Name, std::string CalibrationFile)
  : Name(Name) {
  LOG(INIT, Sev::Info, "Loading calibration file {}", CalibrationFile);

  try {
    root = from_json_file(CalibrationFile);
  } catch (...) {
    LOG(INIT, Sev::Error, "Caen calibration - error: Invalid Json file: {}",
        CalibrationFile);
    throw std::runtime_error("Invalid Json file for CAEN");
  }
}

void CDCalibration::parseCalibration() {
  consistencyCheck();
}


nlohmann::json CDCalibration::getObjectAndCheck(nlohmann::json JsonObject, std::string Property) {
  nlohmann::json JsonObj = JsonObject[Property];
  if (not JsonObj.is_object()) {
    throw std::runtime_error(fmt::format("rethrow: '{}' does not return a json object", Property));

  }
  return JsonObj;
}


void CDCalibration::consistencyCheck() {
  nlohmann::json Calibration = getObjectAndCheck(root, "Calibration");
  std::string Instrument = Calibration["instrument"];
  if (Instrument != Name) {
    throw std::runtime_error("Instrument name error");
  }

  Parms.Groups = Calibration["groups"];
  Parms.GroupSize = Calibration["groupsize"];
  Parms.Amplitudes = Calibration["amplitudes"];
  Parms.Pixellation = Calibration["pixellation"];
  printf("%s, %d, %d, %d, %d\n", Name.c_str(), Parms.Groups, Parms.GroupSize,
        Parms.Amplitudes, Parms.Pixellation);
  int Pixels = Parms.Groups * Parms.GroupSize * Parms.Pixellation;
  printf("NUMBER OF PIXELS: %d\n", Pixels);

  auto DefaultIntervals = Calibration["default intervals"];

  if (DefaultIntervals.size() != (unsigned int)(Parms.GroupSize * 2)) {
    throw std::runtime_error(fmt::format("Interval array error: expected {} entries, got {}",
                 Parms.GroupSize * 2, DefaultIntervals.size()));
  }

  auto ParameterVector = Calibration["Parameters"];
  if (ParameterVector.size() != (unsigned int)(Parms.Groups)) {
    throw std::runtime_error(fmt::format("Calibration table error: expected {} entries, got {}",
           Parms.Groups, ParameterVector.size()));
  }
}

} // namespace Caen
