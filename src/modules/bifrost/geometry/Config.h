// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
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

namespace Bifrost {
class Config {
public:
  Config() {}

  const int MaxValidRing{2};
  const int MaxValidFEN{0};
  const int MaxValidTube{14};

  uint32_t MaxPulseTimeDiffNS{0xffffffff};

private:
};
} // namespace Bifrost
