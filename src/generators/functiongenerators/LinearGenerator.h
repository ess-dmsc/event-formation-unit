// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generates linear values based on a given gradient.
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <generators/functiongenerators/FunctionGenerator.h>
#include <random>

///
/// @class LinearGenerator
/// @brief A class that generates linear values based on a given gradient.
///
class LinearGenerator : public FunctionGenerator {
public:
  LinearGenerator(double MaxX, double gradient, uint32_t offset = 0.0)
      : Offset(offset), BinWidth{static_cast<float>(MaxX / Bins)},
        ValueBins(new uint32_t[Bins]) {

    for (int i = 0; i < Bins; i++) {
      ValueBins[i] = static_cast<uint32_t>(i * gradient);
    }
  };

  ///
  /// \brief Get the value at a given position.
  ///
  double getDistValue(const double &Pos) override {

    int binIndex = static_cast<int>(Pos / BinWidth);

    return Offset + ValueBins[binIndex];
  }

public:
  int Bins{512};
  uint32_t Offset;
  float BinWidth{0.0};
  uint32_t *ValueBins;
};

// GCOVR_EXCL_STOP
