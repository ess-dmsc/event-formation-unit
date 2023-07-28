// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read calibrations from file
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <modules/caen/geometry/Calibration.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

///
Calibration::Calibration() {}

/// loads calibration from file
Calibration::Calibration(std::string CalibrationFile) {
  LOG(INIT, Sev::Info, "Loading calibration file {}", CalibrationFile);

  try {
    root = from_json_file(CalibrationFile);
  } catch (...) {
    LOG(INIT, Sev::Error, "Caen calibration - error: Invalid Json file: {}",
        CalibrationFile);
    throw std::runtime_error("Invalid Json file for CAEN");
  }
}

///\brief load BIFROST specific calibration parameters
void Calibration::loadBifrostParameters() {
  try {
    auto BifrostCalibJson = root["BifrostCalibration"];

    auto Intervals = BifrostCalibJson["Intervals"];
    for (auto &Triplet : Intervals) {
      uint32_t TripletId = Triplet[0].get<uint32_t>();
      if (TripletId > 44) {
        XTRACE(INIT, ERR, "Invalid TripletId %u (Max is 44)", TripletId);
        throw std::runtime_error("Invalid TripletId (max 44)");
      }
      float p0 = Triplet[1].get<float>();
      float p1 = Triplet[2].get<float>();
      float p2 = Triplet[3].get<float>();
      float p3 = Triplet[4].get<float>();
      float p4 = Triplet[5].get<float>();
      float p5 = Triplet[6].get<float>();
      std::vector<float> Calib{p0, p1, p2, p3, p4, p5};
      BifrostCalibration.TripletCalib[TripletId] = Calib;
    }

  } catch (...) {
    throw std::runtime_error("Invalid BIFROST calibration");
  }

  MaxPixelId = 45 * 3 * 100; ///\todo I know
}

void Calibration::loadLokiParameters() {
  try {
    auto CaenCalibration = root["LokiCalibration"];

    uint32_t NTubes = CaenCalibration["ntubes"].get<uint32_t>();
    uint32_t NStraws = CaenCalibration["nstraws"].get<uint32_t>();

    NumberOfStraws = NTubes * NStraws;
    StrawResolution = CaenCalibration["resolution"].get<uint16_t>();

    auto Pols = CaenCalibration["polynomials"];
    if (Pols.size() != NumberOfStraws) {
      throw std::runtime_error("Straw number mismatch in calibration file");
    }

    XTRACE(INIT, DEB, "ntubes %u, nstraws %u, tot straws %u", NTubes, NStraws,
           NumberOfStraws);

    uint32_t ExpectedStraw{0}; // to check that straws are in ascending order

    for (auto &e : Pols) {
      if (e.size() != 5) {
        throw std::runtime_error("Wrong number of coefficients");
      }

      uint16_t Straw = e[0].get<uint16_t>();
      XTRACE(INIT, DEB, "Straw: %u", Straw);

      if (Straw != ExpectedStraw) {
        throw std::runtime_error("Unexpected straw id");
      }

      double a = e[1].get<double>();
      double b = e[2].get<double>();
      double c = e[3].get<double>();
      double d = e[4].get<double>();
      std::vector<double> cal{a, b, c, d};
      XTRACE(INIT, DEB, "Calibration entry - Straw %d: %g %g %g %g", Straw, a,
             b, c, d);
      StrawCalibration.push_back(cal);
      ExpectedStraw++;
    }
    MaxPixelId = NumberOfStraws * StrawResolution;

  } catch (...) {
    throw std::runtime_error("Invalid LOKI calibration");
  }
}

/// \brief create a null calibration, or identity mapping
void Calibration::nullCalibration(uint32_t Straws, uint16_t Resolution) {
  if ((Straws <= 7) or (Resolution < 128) or (Resolution > 1024)) {
    throw std::runtime_error("Invalid nullcalibration dimensions");
  }

  NumberOfStraws = Straws;
  StrawResolution = Resolution;
  XTRACE(INIT, INF, "Straws: %d, Resolution: %u", Straws, Resolution);

  std::vector<double> StrawCalib{0, 0, 0, 0};

  for (uint32_t Straw = 0; Straw < Straws; Straw++) {
    StrawCalibration.push_back(StrawCalib);
  }

  MaxPixelId = Straws * Resolution;
}

uint32_t Calibration::strawCorrection(uint32_t StrawId, double Pos) {
  double a = StrawCalibration[StrawId][0];
  double b = StrawCalibration[StrawId][1];
  double c = StrawCalibration[StrawId][2];
  double d = StrawCalibration[StrawId][3];

  double Delta = a + Pos * (b + Pos * (c + Pos * d));

  XTRACE(EVENT, DEB, "straw: %u, pos: %g, delta %g", StrawId, Pos, Delta);
  double CorrectedPos = Pos - Delta;

  if (CorrectedPos < 0) {
    XTRACE(EVENT, INF, "Clamping to low value, straw: %u, pos: %g, delta %g",
           StrawId, Pos, Delta);
    (*Stats.ClampLow)++;
    CorrectedPos = 0;
  }
  if (CorrectedPos > StrawResolution) {
    XTRACE(EVENT, INF, "Clamping to high value, straw: %u, pos: %g, delta %g",
           StrawId, Pos, Delta);
    (*Stats.ClampHigh)++;
    CorrectedPos = StrawResolution - 1;
  }
  return (uint32_t)CorrectedPos;
}
} // namespace Caen
