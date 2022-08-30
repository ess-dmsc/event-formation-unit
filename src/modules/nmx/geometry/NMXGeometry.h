// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief NMX geometry class
/// Based on NMX ICD documen

/// Mapping from digital identifiers to x- and y- coordinates
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <nmx/geometry/Geometry.h>

namespace Nmx {

class NMXGeometry : public Nmx::Geometry {
public:
  uint16_t coord(uint8_t Channel, uint8_t AsicId, uint16_t Offset,
                 bool ReversedChannels) override;
};

} // namespace Nmx
