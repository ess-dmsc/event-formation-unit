// Copyright (C) 2025 - 2026 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief BEER detector base - inherits from CbmBase with BEER-specific config
///
//===----------------------------------------------------------------------===//

#pragma once

#include <modules/cbm/CbmBase.h>
#include <modules/beer/geometry/Config.h>
#include <modules/beer/readout/Parser.h>

namespace beer {

/// BeerBase inherits from CbmBase but uses beer::Config instead of cbm::Config.
class BeerBase : public cbm::CbmBase {
public:
  BeerBase(BaseSettings const &settings);
};

} // namespace beer
