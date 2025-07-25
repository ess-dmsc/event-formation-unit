// Copyright (C) 2024 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Interface for function generators
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once
#include <cstdint>

class FunctionGenerator {
public:
  /// \brief Number of Bins defines the resolution of the distribution function.
  static constexpr uint32_t DEFAULT_BIN_COUNT{512};
  virtual ~FunctionGenerator() {}

  /// \brief Get the value at a given position in the generator function.
  virtual double getValueByPos(double) = 0;

  /// \brief return a parameter value used to generate a distribution function. 
  virtual double getValue() = 0;
  
};

// GCOVR_EXCL_STOP