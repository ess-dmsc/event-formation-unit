/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Get detector configuration from json file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <caen/geometry/PanelGeometry.h>
#include <common/debug/Trace.h>
#include <logical_geometry/ESSGeometry.h>
#include <string>
#include <vector>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

namespace Caen {
class Config {
public:
  Config();

  Config(std::string ConfigFile);

  std::vector<PanelGeometry> Panels;
  std::string InstrumentName;
  uint8_t NPanels{0};              // Caen panels, not logical geometry panels
  uint16_t NTubesTotal{0};         // total number of tubes in instrument
  uint16_t Resolution{0};          /// Resolution along straws
  uint32_t ReadoutConstDelayNS{0}; /// added to readout data timestamp
  uint32_t MaxPulseTimeNS{5 * 71'428'571}; // 5 * 1/14 * 10^9
  uint32_t MaxTOFNS{800000000};
  uint8_t MaxRing{0};
};
} // namespace Caen
