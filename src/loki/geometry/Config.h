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

  std::vector<PanelGeometry> Panels;
  uint8_t NPanels{0}; // Loki panels, not logical geometry panels
};
} // namespace
