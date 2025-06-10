// Copyright (C) 2023 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <timepix3/geometry/Config.h>

#include <common/JsonFile.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

Config::Config() {}

Config::Config(const std::string &ConfigFile)
  : Configurations::Config(ConfigFile) {
  XTRACE(INIT, DEB, "Loading json file");
  loadFromFile();
  XTRACE(INIT, DEB, "Loaded json file");

  setMask(LOG | CHECK);
  assign("Detector", InstrumentName);

  if (InstrumentName != "timepix3") {
    LOG(INIT, Sev::Error, "InstrumentName mismatch");
    throw std::runtime_error(
        "Inconsistent Json file - invalid name, expected timepix3");
  }

  assign("XResolution", XResolution);
  assign("YResolution", YResolution);
  assign("ParallelThreads", ParallelThreads);

  try {
    ScaleUpFactor = assign("ScaleUpFactor", ScaleUpFactor);
    LOG(INIT, Sev::Info,
        "Camera image resolution ({}X{}) scaled up with factor {} to super "
        "resolution of ({}X{})",
        XResolution, YResolution, ScaleUpFactor, XResolution * ScaleUpFactor,
        YResolution * ScaleUpFactor);
  } catch (...) {
    LOG(INIT, Sev::Info,
        "Using default ScaleUpFactor = {}, super resolution not applied",
        ScaleUpFactor);
  }

  setMask(LOG);
  assign("MaxTimeGapNS", MaxTimeGapNS);
  assign("MinEventTimeSpanNS", MinEventTimeSpanNS);
  assign("FrequencyHz", FrequencyHz);
  assign("MinEventSizeHits", MinEventSizeHits);
  assign("MinimumToTSum", MinimumToTSum);
  assign("MaxCoordinateGap", MaxCoordinateGap);
}

} // namespace Timepix3
