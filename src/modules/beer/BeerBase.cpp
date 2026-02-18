// Copyright (C) 2025 - 2026 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief BEER detector base implementation
///
//===----------------------------------------------------------------------===//

#include <modules/beer/BeerBase.h>

namespace beer {

BeerBase::BeerBase(BaseSettings const &settings) : cbm::CbmBase(settings) {
  // Replace with BEER-specific config and parser
  CbmConfiguration = std::make_unique<Config>(EFUSettings.ConfigFile);
  CbmParser = std::make_unique<Parser>();
}

} // namespace beer
