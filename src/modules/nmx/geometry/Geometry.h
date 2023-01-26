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
  ///
  ///\brief calculate coordinate value, in either plane
  ///
  ///\param Channel integer between 0 and 63 identifying the channel
  ///         a readout has come from.
  ///\param AsicId 0 or 1, identifying the asic a readout is from
  ///\param Offset offset from 0
  ///\param ReversedChannels boolean determining if channel order
  ///         is reversed, ie. top half of a panel in X plane,
  ///         or right side of a panel in Y plane.
  ///\return uint16_t
  ///
  virtual uint16_t coord(uint8_t Channel, uint8_t AsicId, uint16_t Offset,
                         bool ReversedChannels) = 0;
  static constexpr uint16_t InvalidCoord = 65535;
};
} // namespace Nmx
