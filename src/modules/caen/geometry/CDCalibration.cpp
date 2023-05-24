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
  printf("Number of pixels: %d\n", Pixels);

  auto DefaultIntervals = Calibration["default intervals"];

  if (DefaultIntervals.size() != (unsigned int)(Parms.GroupSize * 2)) {
    auto msg = fmt::format("Interval array error: expected {} entries, got {}",
                 Parms.GroupSize * 2, DefaultIntervals.size());
    throw std::runtime_error(msg);
  }

  auto ParameterVector = Calibration["Parameters"];
  if (ParameterVector.size() != (unsigned int)(Parms.Groups)) {
    throw std::runtime_error(fmt::format("Calibration table error: expected {} entries, got {}",
           Parms.Groups, ParameterVector.size()));
  }

  int Index{0};
  for (auto & Parm : ParameterVector) {
    int GroupIndex = Parm["groupindex"];
    if (GroupIndex != Index) {
      auto msg = fmt::format("Index error: expected {}, got {}", Index, GroupIndex);
      throw std::runtime_error(msg);
    }
    validateIntervals(Index, Parm);
    validatePolynomials(Index, Parm);
    Index++;
  }
}


void CDCalibration::validateIntervals(int Index, nlohmann::json Parameter) {
  std::vector<float> Intervals = Parameter["intervals"];
  if (Intervals.size() != (unsigned int)(Parms.GroupSize * 2)) {
    auto msg = fmt::format("Groupindex {} - interval array error: expected {} entries, got {}",
                 Index, Parms.GroupSize * 2, Intervals.size());
    throw std::runtime_error(msg);
  }
  float PreviousPos{-1.0};
  int IntervalIndex{0};
  for (auto & Pos : Intervals) {
    if( (Pos < 0.0) or (Pos > 1.0)) {
      auto msg = fmt::format("Groupindex {}, Intervalpos {} - bad value {}",
                   Index, IntervalIndex, Pos);
      throw std::runtime_error(msg);
    }
    if(Pos <= PreviousPos) {
      auto msg = fmt::format("Groupindex {}, Intervalindex {} - value {} is smaller than previous {}",
                   Index, IntervalIndex, Pos, PreviousPos);
      throw std::runtime_error(msg);
    }
    PreviousPos = Pos;
    IntervalIndex++;
  }
}

void CDCalibration::validatePolynomials(int Index, nlohmann::json Parameter) {
  std::vector<std::vector<float>> Polynomials = Parameter["polynomials"];
  if (Polynomials.size() != (unsigned int)Parms.GroupSize) {
    auto msg = fmt::format("Groupindex {} bad groupsize: expected {}, got {}",
           Index, Parms.GroupSize, Polynomials.size());
    throw std::runtime_error(msg);
  }
  for (auto & Coefficients : Polynomials) {
    if (Coefficients.size() != 4) {
      auto msg = fmt::format("Groupindex {} coefficient error: expected 4, got {}",
        Index, Coefficients.size());
      throw std::runtime_error(msg);
    }
  }

}

} // namespace Caen
