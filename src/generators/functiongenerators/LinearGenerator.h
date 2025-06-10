// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generates linear values based on a given Gradient.
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <generators/functiongenerators/FunctionGenerator.h>
#include <memory>
#include <stdexcept>
#include <vector>
#include <cmath>

/// \class LinearGenerator
/// \brief A class that generates linear values based on a given Gradient. The generator
/// will return readout values with specific intervals and restart when maximum readout value
/// is reached
/// 
class LinearGenerator : public FunctionGenerator {
public:
  /// \brief Linear generator is working in nano seconds
  static constexpr double TimeDurationUnit{1E9};

  ///
  /// \brief Distribution constructor based on the rotation frequency of the target
  /// wheel. 
  ///
  /// \param MaxX maximum duration of the pulse in nano seconds
  /// \param Bins The number of bins in the distribution. We always use the
  /// absolute value of Bins.
  /// \param Gradient readout interval in nanoseconds
  /// \param Offset value that will be added to getValueByIndex.
  ///
  explicit LinearGenerator(double MaxX, uint32_t Bins, double Gradient, uint32_t Offset = 0.0)
    : PulseDuration{MaxX}
    , gradient{Gradient} 
    , offset(Offset) 
    , BinWidth{static_cast<double>(MaxX / Bins)}{
  };

  ///
  /// \brief Distribution constructor based on the rotation frequency of the target
  /// wheel. 
  ///
  /// \param MaxX maximum duration of the pulse in nano seconds
  /// \param Gradient readout interval in nanoseconds
  /// \param Offset value that will be added to getValueByIndex.
  ///
  explicit LinearGenerator(double MaxX, double Gradient, uint32_t Offset = 0.0)
      : LinearGenerator{MaxX, DEFAULT_BIN_COUNT, Gradient, Offset} {
  };

  ///
  /// \brief Distribution constructor based on the rotation frequency of the target
  /// wheel. 
  ///
  /// \param Frequency pulse frequency
  /// \param Gradient readout interval in nanoseconds
  /// \param Offset value that will be added to getValueByIndex.
  ///
  explicit LinearGenerator(uint16_t frequency, double Gradient, uint32_t Offset = 0.0)
      : LinearGenerator {TimeDurationUnit / frequency, Gradient, Offset} {
  };

  /// \brief Get the value at a given position.
  ///
  double getValueByIndex(const double &Pos) override {

    int binIndex = static_cast<int>(Pos / BinWidth);

    return offset + binIndex * BinWidth;
  }

  /// \brief return an internal incremented value. The value increments with 
  /// gradient value per call and will not be larger than MaxM. 
  /// Since this method work with an internal counter an instance of the class can
  /// have unpredicted behaviour if invoked in different places.
  double getValue() override { 
    double result = CurrentReadout;
    CurrentReadout += gradient;
    if (CurrentReadout > PulseDuration)
      CurrentReadout = 0.0;
    return std::ceil(result / BinWidth) * BinWidth;
  }

private:
  // An internal readout counter. 
  double CurrentReadout{0.0};
  // The maximum value a readout can be with in a pulse 
  double PulseDuration{0.0};

  double gradient;
  uint32_t offset;
  double BinWidth{0.0};
};

// GCOVR_EXCL_STOP
