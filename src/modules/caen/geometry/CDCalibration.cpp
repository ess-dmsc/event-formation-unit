// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of Charge Division calibration (CDCalibration)
/// Implementing ideas from ownCloud folder
/// 'DM/detectors/02 instruments/01 common/03 Calibration/Charge division'
///
/// \brief using nlohmann json to parse calibrations read from file
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <modules/caen/geometry/CDCalibration.h>
#include <modules/caen/geometry/Interval.h>
#include <utility>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

/// loads calibration from file
CDCalibration::CDCalibration(const std::string &Name, const std::string &CalibrationFile)
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

double CDCalibration::posCorrection(int Group, int Unit, double Pos) {
  std::vector<double> &Pols = Calibration[Group][Unit];
  double a = Pols[0];
  double b = Pols[1];
  double c = Pols[2];
  double d = Pols[3];

  double Delta = a + Pos * (b + Pos * (c + Pos * d));
  XTRACE(EVENT, DEB, "group %d, unit %d, pos: %g, delta %g", Group, Unit, Pos,
         Delta);
  double CorrectedPos = Pos - Delta;
  XTRACE(EVENT, DEB, "CorrectedPos %g", CorrectedPos);

  if (CorrectedPos < 0.0) {
    XTRACE(EVENT, INF, "Clamping to low value, pos: %g, delta %g", Pos, Delta);
    Stats.ClampLow++;
    CorrectedPos = 0.0;
  }
  if (CorrectedPos > 1.0) {
    XTRACE(EVENT, INF, "Clamping to high value, pos: %g, delta %g", Pos, Delta);
    Stats.ClampHigh++;
    CorrectedPos = 1.0;
  }

  return CorrectedPos;
}

///\brief
int CDCalibration::getUnitId(int GroupIndex, double GlobalPos) {
  XTRACE(EVENT, DEB, "GroupIndex %u GlobalPos %f", GroupIndex, GlobalPos);

  if (GroupIndex >= Parms.Groups) {
    XTRACE(EVENT, WAR, "Provided GroupIndex %d > config (%d)", GroupIndex, Parms.Groups);
    Stats.GroupErrors++;
    return -1;
  }
  auto &GroupIntervals = Intervals[GroupIndex];

  int Unit;
  for (Unit = 0; Unit < (int)GroupIntervals.size(); Unit++) {
    double Min =
        std::min(GroupIntervals[Unit].first, GroupIntervals[Unit].second);
    double Max =
        std::max(GroupIntervals[Unit].first, GroupIntervals[Unit].second);
    if ((GlobalPos >= Min) and (GlobalPos <= Max)) {
      return Unit;
    }
  }
  Stats.OutsideInterval++;
  return -1;
}

///\brief Use a two-pass approach. One pass to validate as much as possible,
/// then a second pass to populate calibration table
void CDCalibration::parseCalibration() {
  consistencyCheck(); // first pass for checking
  loadCalibration();  // second pass to populate table
}

void CDCalibration::consistencyCheck() {
  XTRACE(INIT, DEB, "Get Calibration object");
  nlohmann::json Calibration = getObjectAndCheck(root, "Calibration");
  XTRACE(INIT, DEB, "Get instrument name");
  std::string Instrument = Calibration["instrument"];
  if (Instrument != Name) {
    throwException("Instrument name error");
  }

  Parms.Groups = Calibration["groups"];
  Parms.GroupSize = Calibration["groupsize"];

  XTRACE(INIT, ALW, "Parms: %s, %d, %d", Name.c_str(), Parms.Groups,
         Parms.GroupSize);

  XTRACE(INIT, DEB, "Get Parameter object");
  auto ParameterVector = Calibration["Parameters"];
  if (ParameterVector.size() != (unsigned int)(Parms.Groups)) {
    throw std::runtime_error(
        fmt::format("Calibration table error: expected {} entries, got {}",
                    Parms.Groups, ParameterVector.size()));
  }

  int Index{0};
  for (auto &Parm : ParameterVector) {
    XTRACE(INIT, DEB, "Get groupindex object %d", Index);
    int GroupIndex = Parm["groupindex"];
    if (GroupIndex != Index) {
      Message =
          fmt::format("Index error: expected {}, got {}", Index, GroupIndex);
      throwException(Message);
    }
    validateIntervals(Index, Parm);
    validatePolynomials(Index, Parm);
    Index++;
  }
}

void CDCalibration::loadCalibration() {
  int Polynomials{0};

  auto ParameterVector = root["Calibration"]["Parameters"];
  if ((int)ParameterVector.size() != Parms.Groups) {
    Message = fmt::format("Groupsize mismatch: {} specified, {} received",
      Parms.Groups, ParameterVector.size());
    throwException(Message);
  }
  for (auto &Group : ParameterVector) {
    auto GroupIntervals =
        Group["intervals"].get<std::vector<std::pair<double, double>>>();
    Intervals.push_back(GroupIntervals);

    auto GroupPolys =
        Group["polynomials"].get<std::vector<std::vector<double>>>();
    Calibration.push_back(GroupPolys);
    Polynomials += Parms.GroupSize;
  }
  XTRACE(INIT, ALW, "Loaded %d polynomials from %d groups", Polynomials,
         Parms.Groups);


}

nlohmann::json CDCalibration::getObjectAndCheck(nlohmann::json JsonObject,
                                                std::string Property) {
  nlohmann::json JsonObj = JsonObject[Property];
  if (not JsonObj.is_object()) {
    Message = fmt::format("'{}' does not return a json object", Property);
    throwException(Message);
  }
  return JsonObj;
}

void CDCalibration::validateIntervals(int Index, nlohmann::json Parameter) {
  std::vector<std::pair<double, double>> Intervals = Parameter["intervals"];
  if (Intervals.size() != (unsigned int)(Parms.GroupSize)) {
    Message = fmt::format(
        "Groupindex {} - interval array error: expected {} entries, got {}",
        Index, Parms.GroupSize, Intervals.size());
    throwException(Message);
  }

  // check for interval overlaps
  if (Interval::overlaps(Intervals)) {
    Message = fmt::format("Groupindex {} has overlapping intervals", Index);
    throwException(Message);
  }

  int IntervalIndex{0};

  for (auto &Interval : Intervals) {
    if (not inUnitInterval(Interval)) {
      Message =
          fmt::format("Groupindex {}, Intervalpos {} - bad range [{}; {}]",
                      Index, IntervalIndex, Interval.first, Interval.second);
      throwException(Message);
    }
    IntervalIndex++;
  }
}

void CDCalibration::validatePolynomials(int Index, nlohmann::json Parameter) {
  std::vector<std::vector<double>> Polynomials = Parameter["polynomials"];
  if (Polynomials.size() != (unsigned int)Parms.GroupSize) {
    Message = fmt::format("Groupindex {} bad groupsize: expected {}, got {}",
                          Index, Parms.GroupSize, Polynomials.size());
    throwException(Message);
  }
  for (auto &Coefficients : Polynomials) {
    if (Coefficients.size() != 4) {
      Message =
          fmt::format("Groupindex {} coefficient error: expected 4, got {}",
                      Index, Coefficients.size());
      throwException(Message);
    }
  }
}

bool CDCalibration::inUnitInterval(std::pair<double, double> &Pair) {
  return ((Pair.first >= 0.0) and (Pair.first <= 1.0) and
          (Pair.second >= 0.0) and (Pair.second <= 1.0));
}

void CDCalibration::throwException(const std::string &Message) {
  XTRACE(INIT, ERR, "%s", Message.c_str());
  LOG(INIT, Sev::Error, Message);
  throw std::runtime_error(Message);
}

} // namespace Caen
