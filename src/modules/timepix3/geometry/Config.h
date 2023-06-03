// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Get detector configuration from json file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <logical_geometry/ESSGeometry.h>
#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {
class Config {
public:
  Config();

  Config(std::string ConfigFile);

  std::string InstrumentName;
  uint8_t NPanels{0};      // Timepix3 panels, not logical geometry panels
  uint16_t NTubesTotal{0}; // total number of tubes in instrument
  uint16_t XResolution{0}; /// Resolution along x axis
  uint16_t YResolution{0}; /// Resolution along y axis
  uint32_t ReadoutConstDelayNS{0};         /// added to readout data timestamp
  uint32_t MaxPulseTimeNS{5 * 71'428'571}; // 5 * 1/14 * 10^9
  uint32_t MaxTOFNS{800000000};
  
  uint32_t MaxTimeGapNS{1};
  uint32_t MinEventSizeHits{1};
  uint32_t MinimumToTSum{0};
  uint16_t MaxCoordinateGap{5};

};
} // namespace Timepix3
