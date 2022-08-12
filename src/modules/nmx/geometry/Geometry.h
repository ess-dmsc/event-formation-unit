// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief NMX geometry class
///
/// Mapping from digital identifiers to x- and y- coordinates
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>

#include <string>
#include <utility>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Nmx {

class Geometry {
public:
  virtual uint16_t coord(uint8_t Channel, uint8_t AsicId, uint16_t Offset,
                         bool ReversedChannels) = 0;
  static constexpr uint16_t InvalidCoord = 65535;
};
} // namespace Nmx
