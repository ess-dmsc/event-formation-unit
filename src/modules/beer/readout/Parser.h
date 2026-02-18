// Copyright (C) 2025 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ESS readout parser for BEER instrument - only supports EVENT_2D
///
//===----------------------------------------------------------------------===//

#pragma once

#include <modules/cbm/readout/Parser.h>

namespace beer {

///
/// \class Parser
/// \brief BEER-specific parser extending CBM Parser
///
/// This class reuses the CBM parsing infrastructure but only accepts
/// EVENT_2D readout types. All other types will be rejected during parsing.
///
class Parser : public cbm::Parser {
public:
  Parser() = default;
  ~Parser() override = default;

protected:
  ///
  /// \brief Validate the readout type - only EVENT_2D is allowed for BEER
  /// \param Type The readout type to validate
  /// \return true only if the type is EVENT_2D
  ///
  bool isValidType(uint8_t Type) const override;
};

} // namespace beer
