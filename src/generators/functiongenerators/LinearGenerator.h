// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generates linear values based on a given Gradient.
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <cmath>
#include <cstdint>
#include <generators/functiongenerators/FunctionGenerator.h>
#include <memory>
#include <stdexcept>
#include <vector>

/// \class LinearGenerator
/// \brief  Generates linearly increasing values based on a given maximum X
/// value, number of bins, and gradient. The generator supports both fixed and
/// frequency-based initialization. The generator cann ot handle negative
/// X values and will throw an exception if such values are provided.
///
class LinearGenerator : public FunctionGenerator {
public:
  /// \brief Linear generator is working in nano seconds
  static constexpr double TIME_UNIT_NS{1E9};

  /// \brief LinearGenerator generates a function where Y values are
  /// lineraly increasing while X values also increasing from 0 towards the
  /// maximum X value. X values are increase according to the number of bins.
  /// Negative X values are not supported.
  /// \param MaxXValue The maximum X value for the generator.
  /// \param numBins The number of bins to divide the X value into.
  /// \param Gradient The gradient value for the linear function.
  /// \param XZeroOffset The X zero offset, used to shift the start to ensure we
  /// do not generate values at the bin edges.
  /// \throws std::invalid_argument if MaxXValue is negative.
  /// \throws std::invalid_argument if Gradient is negative.
  /// \throws std::invalid_argument if numBins is zero.
  ///
  explicit LinearGenerator(double MaxXValue, uint32_t numBins, double Gradient,
                           double XZeroOffset = 0.0)
      : PulseDuration{MaxXValue}, numBins(numBins), gradient(Gradient),
        XZeroOffset(XZeroOffset) {
    if (numBins == 0) {
      throw std::invalid_argument("Number of bins cannot be zero");
    }
    if (MaxXValue < 0) {
      throw std::invalid_argument("Maximum X value cannot be negative");
    }
    if (Gradient < 0) {
      throw std::invalid_argument("Gradient cannot be negative");
    }
    binWidth = ceil(PulseDuration / numBins);
  }

  /// \brief LinearGenerator generates a function where Y values are
  /// lineraly increasing while X values also increasing from 0 towards the
  /// maximum X value. X values are increase according to the number of bins.
  /// Negative X values are not supported.
  /// \param MaxXValue The maximum X value for the generator.
  /// \param numBins The number of bins to divide the X value into.
  /// \throws std::invalid_argument if MaxXValue is negative.
  /// \throws std::invalid_argument if numBins is zero.
  /// \note: Start of  the function is shifted by half of the bin width to
  /// ensure we do not generate values at the bin edges.
  ///
  explicit LinearGenerator(double MaxXValue, uint32_t numBins)
      : LinearGenerator(MaxXValue, numBins, ceil(MaxXValue / numBins),
                        ceil(MaxXValue / numBins / 2)) {}

  /// \brief LinearGenerator generates a function where Y values are
  /// lineraly increasing while X values also increasing from 0 towards the
  /// maximum X value. X values are increase according to the number of bins.
  /// Negative X values are not supported.
  /// \param frequency The frequency in Hz for the generator, which is used to
  /// calculate the maximum X value as TIME_UNIT_NS / frequency.
  /// \param numBins The number of bins to divide the X value into.
  /// \param Gradient The gradient value for the linear function.
  /// \throws std::invalid_argument if frequency is zero.
  /// \throws std::invalid_argument if Gradient is negative.
  /// \throws std::invalid_argument if numBins is zero.
  ///
  explicit LinearGenerator(uint16_t frequency, uint32_t numBins,
                           double gradient) {
    if (frequency == 0) {
      throw std::invalid_argument("Frequency cannot be zero");
    }

    *this = LinearGenerator(ceil(TIME_UNIT_NS / frequency), numBins, gradient);
  }

  /// \brief LinearGenerator generates a function where Y values are
  /// lineraly increasing while X values also increasing from 0 towards the
  /// maximum X value. X values are increase according to the number of bins.
  /// Negative X values are not supported.
  /// \param frequency The frequency in Hz for the generator, which is used to
  /// calculate the maximum X value as TIME_UNIT_NS / frequency.
  /// \param numBins The number of bins to divide the X value into.
  /// \param Gradient The gradient value for the linear function.
  /// \throws std::invalid_argument if frequency is zero.
  /// \throws std::invalid_argument if numBins is zero.
  /// \note: Start of  the function is shifted by half of the bin width to
  /// ensure we do not generate values at the bin edges.
  ///
  explicit LinearGenerator(uint16_t frequency, uint32_t numBins) {

    if (frequency == 0) {
      throw std::invalid_argument("Frequency cannot be zero");
    }

    *this = LinearGenerator(ceil(TIME_UNIT_NS / frequency), numBins);
  }

  /// \brief Get the value at a given position.
  ///
  double getValueByPos(double Pos) override {
    if (Pos < 0) {
      throw std::invalid_argument("Position cannot be negative");
    }
    auto index = static_cast<uint32_t>(Pos / binWidth);
    if (index >= numBins)
      index = 0;
    return gradient * index;
  }

  /// \brief return an internal incremented value. The value increments with
  /// gradient value per call and will not be larger than MaxM.
  /// Since this method work with an internal counter an instance of the class
  /// can have unpredicted behaviour if invoked in different places.
  double getValue() override {
    if (CurrentReadout >= numBins) {
      CurrentReadout = 0.0;
    }
    return gradient * CurrentReadout++ + XZeroOffset;
  }

private:
  // An internal readout counter.
  double CurrentReadout{0.0};
  // The maximum value a readout can be with in a pulse
  double PulseDuration{0.0};
  uint32_t numBins{0};
  double gradient;
  double binWidth{0.0};
  double XZeroOffset{0.0}; // The X zero offset, used to shift the start to
                           // ensure we do not generate values at the bin edges
};

// GCOVR_EXCL_STOP
