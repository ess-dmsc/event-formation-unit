// Copyright (C) 2025 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of BEER instrument - only EVENT_2D supported
///
//===----------------------------------------------------------------------===//

#include <modules/beer/readout/Parser.h>
#include <modules/cbm/CbmTypes.h>

namespace beer {

bool Parser::isValidType(uint8_t Type) const {
  // BEER only supports EVENT_2D readout type
  return Type == cbm::CbmType::EVENT_2D;
}

} // namespace beer
