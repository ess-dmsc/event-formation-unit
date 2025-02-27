// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3 readouts for multi-blade based setups
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <generators/essudpgen/ReadoutGeneratorBase.h>

namespace Freia {

class MultiBladeGenerator : public ReadoutGeneratorBase {
public:
  MultiBladeGenerator() : ReadoutGeneratorBase(ESSReadout::Parser::DetectorType::FREIA) {}

protected:
  void generateData() override;

  const uint32_t TimeToFirstReadout{1000};
};

} // namespace Freia

// GCOVR_EXCL_STOP
