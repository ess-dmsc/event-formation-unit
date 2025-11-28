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

  setMask(Flags::LOG | CHECK);
  assign("Detector", InstrumentName);

  if (InstrumentName != "timepix3") {
    LOG(INIT, Sev::Error, "InstrumentName mismatch");
    throw std::runtime_error(
        "Inconsistent Json file - invalid name, expected timepix3");
  }
  assign("XResolution", XResolution);
  assign("YResolution", YResolution);
  assign("ParallelThreads", ParallelThreads);

  setMask(Flags::LOG);
  assign("ScaleUpFactor", ScaleUpFactor);
  assign("MaxTimeGapNS", MaxTimeGapNS);
  assign("MinEventTimeSpanNS", MinEventTimeSpanNS);
  assign("FrequencyHz", FrequencyHz);
  assign("MinEventSizeHits", MinEventSizeHits);
  assign("MinimumToTSum", MinimumToTSum);
  assign("MaxCoordinateGap", MaxCoordinateGap);
}

} // namespace Timepix3
