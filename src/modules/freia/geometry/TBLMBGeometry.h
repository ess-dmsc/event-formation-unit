// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief TBLMB geometry class
///
/// Mapping from digital identifiers to x- and y- coordinates
//===----------------------------------------------------------------------===//

#pragma once

#include <freia/geometry/AmorGeometry.h>
#include <freia/geometry/Config.h>
#include <freia/Counters.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Freia {

class TBLMBGeometry final : public AmorGeometry {
public:
	TBLMBGeometry(Statistics &Stats, Config &Cfg) : AmorGeometry(Stats, Cfg) {}

};

} // namespace Freia
