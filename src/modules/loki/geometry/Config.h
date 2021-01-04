/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Get detector configuration from json file
///
//===----------------------------------------------------------------------===//


#pragma once

#include <common/Trace.h>
#include <logical_geometry/ESSGeometry.h>
#include <loki/geometry/PanelGeometry.h>
#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Loki {
class Config {
public:

  Config();

  Config(std::string ConfigFile);

  uint32_t getMaxPixel() { return Pixels; }

  std::vector<PanelGeometry> Panels;
  uint8_t NPanels{0}; // Loki panels, not logical geometry panels
  uint16_t NTubesTotal{0}; // total number of tubes in instrument
  uint16_t Resolution{0}; /// Resolution along straws
  ESSGeometry * Geometry;

private:
  uint32_t Pixels{0};
};
} // namespace
