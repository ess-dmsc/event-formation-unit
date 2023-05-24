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
#include <vector>

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
    Message = fmt::format("Caen calibration - error: Invalid Json file: {}",
        CalibrationFile);
    throwException(Message);
  }
}


void CDCalibration::parseCalibration() {
  consistencyCheck();
}


nlohmann::json CDCalibration::getObjectAndCheck(nlohmann::json JsonObject, std::string Property) {
  nlohmann::json JsonObj = JsonObject[Property];
  if (not JsonObj.is_object()) {
    Message = fmt::format("'{}' does not return a json object", Property);
    throwException(Message);
  }
  return JsonObj;
}


void CDCalibration::consistencyCheck() {
  nlohmann::json Calibration = getObjectAndCheck(root, "Calibration");
  std::string Instrument = Calibration["instrument"];
  if (Instrument != Name) {
    throwException("Instrument name error");
  }

  Parms.Groups = Calibration["groups"];
  Parms.GroupSize = Calibration["groupsize"];
  Parms.Amplitudes = Calibration["amplitudes"];
  Parms.Pixellation = Calibration["pixellation"];
  XTRACE(INIT, ALW, "Parms: %s, %d, %d, %d, %d", Name.c_str(), Parms.Groups, Parms.GroupSize,
        Parms.Amplitudes, Parms.Pixellation);

  MaxPixelId = Parms.Groups * Parms.GroupSize * Parms.Pixellation;
  XTRACE(INIT, ALW, "Calibration - number of pixels: %d", MaxPixelId);

  auto DefaultIntervals = Calibration["default intervals"];

  if (DefaultIntervals.size() != (unsigned int)(Parms.GroupSize * 2)) {
    Message = fmt::format("Interval array error: expected {} entries, got {}",
                 Parms.GroupSize * 2, DefaultIntervals.size());
    throwException(Message);
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
      Message = fmt::format("Index error: expected {}, got {}", Index, GroupIndex);
      throwException(Message);
    }
    validateIntervals(Index, Parm);
    validatePolynomials(Index, Parm);
    Index++;
  }
}


void CDCalibration::validateIntervals(int Index, nlohmann::json Parameter) {
  std::vector<float> Intervals = Parameter["intervals"];
  if (Intervals.size() != (unsigned int)(Parms.GroupSize * 2)) {
    Message = fmt::format("Groupindex {} - interval array error: expected {} entries, got {}",
                 Index, Parms.GroupSize * 2, Intervals.size());
    throwException(Message);
  }
  float PreviousPos{-1.0};
  int IntervalIndex{0};
  for (auto & Pos : Intervals) {
    if( (Pos < 0.0) or (Pos > 1.0)) {
      Message = fmt::format("Groupindex {}, Intervalpos {} - bad value {}",
                   Index, IntervalIndex, Pos);
      throwException(Message);
    }
    if(Pos <= PreviousPos) {
      Message = fmt::format("Groupindex {}, Intervalindex {} - value {} is smaller than previous {}",
                   Index, IntervalIndex, Pos, PreviousPos);
      throwException(Message);
    }
    PreviousPos = Pos;
    IntervalIndex++;
  }
}


void CDCalibration::validatePolynomials(int Index, nlohmann::json Parameter) {
  std::vector<std::vector<float>> Polynomials = Parameter["polynomials"];
  if (Polynomials.size() != (unsigned int)Parms.GroupSize) {
    Message = fmt::format("Groupindex {} bad groupsize: expected {}, got {}",
           Index, Parms.GroupSize, Polynomials.size());
    throwException(Message);
  }
  for (auto & Coefficients : Polynomials) {
    if (Coefficients.size() != 4) {
      Message = fmt::format("Groupindex {} coefficient error: expected 4, got {}",
        Index, Coefficients.size());
      throwException(Message);
    }
  }

}


void CDCalibration::throwException(std::string Message) {
  XTRACE(INIT, ERR, "%s", Message.c_str());
  LOG(INIT, Sev::Error, Message);
  throw std::runtime_error(Message);
}

} // namespace Caen
