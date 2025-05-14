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
#include <memory>

/// \class LinearGenerator
/// \brief A class that generates linear values based on a given gradient.
class LinearGenerator : public FunctionGenerator {
public:
  LinearGenerator(double MaxX, double gradient, uint32_t offset = 0.0)
      : Offset(offset), BinWidth{static_cast<float>(MaxX / Bins)},
        ValueBins(new uint32_t[Bins]) {

    for (int i = 0; i < Bins; i++) {
      ValueBins[i] = static_cast<uint32_t>(i * gradient);
    }
  };

  /// \brief Distribution factory based on the rotation frequency of the target wheel
  static std::shared_ptr<FunctionGenerator> Factory(uint16_t Frequency, double gradient = 1.0, uint32_t offset = 0.0) {
    if (Frequency == 0) {
      throw std::runtime_error("This generator must have a frequency value larger than zero ");
    }
    return std::make_shared<LinearGenerator>( 1000.0 / Frequency, gradient, offset);
  }

  /// \brief Get the value at a given position.
  ///
  double getDistValue(const double &Pos) override {

    int binIndex = static_cast<int>(Pos / BinWidth);

    return Offset + ValueBins[binIndex];
  }

  /// \brief return a random value based on the distribution function
  double getValue() override {
    return 0.0;
  }

public:
  int Bins{512};
  uint32_t Offset;
  float BinWidth{0.0};
  uint32_t *ValueBins;
};

// GCOVR_EXCL_STOP
