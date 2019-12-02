/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read calibrations from file
//===----------------------------------------------------------------------===//

#include <common/Log.h>
#include <loki/geometry/Calibration.h>
#include <common/JsonFile.h>

namespace Loki {

///
Calibration::Calibration() {};

Calibration::Calibration(std::string CalibrationFile) {
  nlohmann::json root = from_json_file(CalibrationFile);
  try {

    auto LokiCalibration = root["LokiCalibration"];

    Mapping = LokiCalibration["Mapping"].get<std::vector<uint32_t>>();

    }
    catch (...) {
      LOG(INIT, Sev::Error, "JSON calibration - error: Invalid Json file: {}", CalibrationFile);
      throw std::runtime_error("Invalid Json file");
      return;
    }

    if (Mapping.size() <= 1) {
      throw std::runtime_error("Invalid Mapping array size");
    }
    MaxPixelId = Mapping.size() - 1;
}


void Calibration::nullCalibration(uint32_t MaxPixels) {
  if (MaxPixels <= 1) {
    throw std::runtime_error("Invalid Mapping array size");
  }
  Mapping.clear();
  Mapping.reserve(MaxPixels);
  for (uint32_t i = 0; i <= MaxPixels; i++) {
    Mapping.push_back(i);
  }

  MaxPixelId = MaxPixels;
}
} // namespace Loki
