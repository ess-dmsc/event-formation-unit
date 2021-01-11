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

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Loki {

///
Calibration::Calibration() {}


/// loads calibration from file
Calibration::Calibration(std::string CalibrationFile) {
  LOG(INIT, Sev::Info, "Loading calibration file {}", CalibrationFile);

  nlohmann::json root = from_json_file(CalibrationFile);
  try {
    auto LokiCalibration = root["LokiCalibration"];

    uint32_t NTubes = LokiCalibration["ntubes"].get<uint32_t>();
    uint32_t NStraws = LokiCalibration["nstraws"].get<uint32_t>();

    NumberOfStraws = NTubes * NStraws;
    StrawResolution = LokiCalibration["resolution"].get<uint16_t>();

    auto Pols = LokiCalibration["polynomials"];
    if (Pols.size() != NumberOfStraws) {
      throw std::runtime_error("Straw number mismatch in calibration file");
    }

    XTRACE(INIT, DEB, "ntubes %u, nstraws %u, tot straws %u",
        NTubes, NStraws, NumberOfStraws);

    uint32_t ExpectedStraw{0}; // to check that straws are in ascending order

    for (auto & e : Pols) {
      if (e["c"].size() != 5) {
        throw std::runtime_error("Invalid polynomial coefficients");
      }

      uint16_t Straw = e["c"][0].get<uint16_t>();
      XTRACE(INIT, DEB, "Straw: %u", Straw);

      if (Straw != ExpectedStraw) {
        throw std::runtime_error("Unexpected straw id");
      }

      double a = e["c"][1].get<double>();
      double b = e["c"][2].get<double>();
      double c = e["c"][3].get<double>();
      double d = e["c"][4].get<double>();
      std::vector<double> cal{a, b, c, d};
      XTRACE(INIT, DEB, "Calibration entry - Straw %d: %g %g %g %g",
        Straw, a, b, c, d);
      StrawCalibration.push_back(cal);
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
  XTRACE(INIT, INF, "Straws: %d, Resolution: %u", Straws, Resolution);

  std::vector<double> StrawCalib{0, 0, 0, 0};

  for (uint32_t Straw = 0; Straw < Straws; Straw++) {
    //XTRACE(INIT, DEB, "Pushing nullcalibration for straw %u", Straw);
    StrawCalibration.push_back(StrawCalib);
  }

  MaxPixelId = Straws * Resolution;
}
} // namespace Loki
