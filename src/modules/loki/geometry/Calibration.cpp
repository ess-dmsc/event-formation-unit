/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read calibrations from file
//===----------------------------------------------------------------------===//

#include <common/Log.h>
#include <common/Trace.h>
#include <loki/geometry/Calibration.h>
#include <common/JsonFile.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

namespace Loki {

///
Calibration::Calibration() {}


// For each polynomial provided (straws in ascending order) generate a
// calibration vector and push it to the StrawMapping vector
void Calibration::addCalibrationEntry(uint32_t Straw, std::vector<float> Polynomial) {
  float a = Polynomial[0];
  float b = Polynomial[1];
  float c = Polynomial[2];
  float d = Polynomial[3];
  std::vector<uint16_t> Calib;
  Calib.reserve(StrawResolution);

  for (uint16_t x = 0; x < StrawResolution; x++) {
    uint16_t newx = a * x * x * x + b * x * x + c * x + d;
    assert(newx >= 0);
    assert(newx < StrawResolution);
    Calib.push_back(newx);
  }
  StrawMapping.push_back(Calib);
  XTRACE(INIT, DEB, "Straw %d - pos 0 -> %d, %d -> %d\n",
          Straw, Calib[0], StrawResolution - 1, Calib[StrawResolution - 1]);
}

/// loads calibration from file
Calibration::Calibration(std::string CalibrationFile) {
  LOG(INIT, Sev::Info, "Loading calibration file {}", CalibrationFile);

  nlohmann::json root = from_json_file(CalibrationFile);
  try {
    auto LokiCalibration = root["LokiCalibration"];
    NumberOfStraws = LokiCalibration["straws"].get<uint32_t>();
    StrawResolution = LokiCalibration["resolution"].get<uint16_t>();

    auto Cals = LokiCalibration["polynomials"];
    if (Cals.size() != NumberOfStraws) {
      throw std::runtime_error("Straw number mismatch in calibration file");
    }

    uint32_t ExpectedStraw{0}; // to check that straws are in ascending order
    for (auto & cal : Cals) {
      auto Straw = cal["straw"].get<uint32_t>();
      if (Straw != ExpectedStraw) {
        throw std::runtime_error("Unexpected straw id");
      }
      auto Poly = cal["poly"].get<std::vector<float>>();
      if (Poly.size() != 4) {
        throw std::runtime_error("Invalid number of polynomial coefficients");
      }

      XTRACE(INIT, DEB, "Calibration entry - Straw %d: %f %f %f %f",
             Straw, Poly[0], Poly[1], Poly[2], Poly[3]);
      addCalibrationEntry(Straw, Poly);
      ExpectedStraw++;
    }
    MaxPixelId = NumberOfStraws * StrawResolution;

  } catch (...) {
    LOG(INIT, Sev::Error, "Loki calibration - error: Invalid Json file: {}", CalibrationFile);
    throw std::runtime_error("Invalid Json file");
    return;
  }
}

/// \brief create a null calibration, or identity mapping
void Calibration::nullCalibration(uint32_t Straws, uint16_t Resolution) {
  if ((Straws <= 7) or (Resolution < 256) or (Resolution > 1024)) {
    throw std::runtime_error("Invalid nullcalibration dimensions");
  }

  NumberOfStraws = Straws;
  StrawResolution = Resolution;

  std::vector<uint16_t> StrawCalib(Resolution);
  for (uint16_t Pos = 0; Pos < Resolution; Pos++) {
    StrawCalib[Pos] = Pos;
  }
  for (uint32_t Straw = 0; Straw < Straws; Straw++) {
    StrawMapping.push_back(StrawCalib);
  }

  MaxPixelId = Straws * Resolution;
}
} // namespace Loki
