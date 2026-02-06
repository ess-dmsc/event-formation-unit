// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Fixed value generator
///
//===----------------------------------------------------------------------===//

#pragma once

#include <generators/functiongenerators/FunctionGenerator.h>

///
/// \class FixedValueGenerator
/// \brief Generator that returns a constant value
///
class FixedValueGenerator : public FunctionGenerator {
public:
  /// \brief Constructor
  /// \param value The fixed value to return
  explicit FixedValueGenerator(uint32_t value) : Value(value) {}

  /// \brief Get value at position (always returns the fixed value)
  /// \param pos Position (ignored)
  /// \return The fixed value
  double getValueByPos(double pos) override;

  /// \brief Get random value (always returns the fixed value)
  /// \return The fixed value
  double getValue() override;

private:
  uint32_t Value;
};
