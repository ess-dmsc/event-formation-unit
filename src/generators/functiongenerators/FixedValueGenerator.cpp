// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of fixed value generator
///
//===----------------------------------------------------------------------===//

#include <generators/functiongenerators/FixedValueGenerator.h>

double FixedValueGenerator::getValueByPos(double pos) {
  (void)pos;  // Unused parameter
  return static_cast<double>(Value);
}

double FixedValueGenerator::getValue() {
  return static_cast<double>(Value);
}
