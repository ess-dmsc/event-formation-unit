// Copyright (C) 2024 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generates random data with a weighted distribution function
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <generators/functiongenerators/FunctionGenerator.h>

#include <memory>
#include <random>
#include <stdexcept>

///
/// The DistributionGenerator class represents a generator for random values
/// based on a distribution function.
///
/// The constructor of this class populates relevant data structures by
/// calculating values for the distribution and integrating them into a
/// cumulative distribution function (not normalized). It also calculates the
/// normalization factor.
///
/// The DistributionGenerator class has the following member variables:
/// - MaxRange: The maximum range of the distribution.
/// - Bins: The number of bins in the distribution.
/// - BinWidth: The width of each bin.
/// - Norm: The normalization factor of the distribution.
/// - Dist: A vector containing the values of the distribution.
/// - CDF: A vector containing the values of the cumulative distribution
/// function.
/// - gen: An object for random number generation using the
/// mersenne_twister_engine with a seed of 1066.
/// - dis: An object for generating uniform real numbers between 0.0 and 1.0.
///
class DistributionGenerator : public FunctionGenerator {
public:
  /// \brief Distribution factory based on the rotation frequency of the target 
  /// wheel. The constructor populates relevant data structures with the default
  /// bin number of 512.
  /// 1) calculate values for distribution and 2) integrate into a cumulative
  /// distribution function (not normalised). Then 3) get normalisation factor.
  /// \param Frequency Rotation frequency of target wheel, must be larger than 
  /// zero.
  explicit DistributionGenerator(uint16_t Frequency);

  /// \brief Distribution factory based on the rotation frequency of the target
  ///  wheel. The constructor populates relevant data structures but with a
  /// custom defined bin number. Bin number defines the resolution of the
  /// distribution function.
  /// 1) calculate values for distribution and 2) integrate into a cumulative
  /// distribution function (not normalised). Then 3) get normalisation factor.
  /// \param Frequency Rotation frequency of target wheel, must be larger than 
  /// zero.
  /// \param Bins The number of bins in the distribution. We always use the
  /// absolute value of Bins.
  explicit DistributionGenerator(uint16_t Frequency, uint32_t Bins);

  /// \brief The constructor populates relevant data structures with the default
  /// bin number of 512.
  /// 1) calculate values for distribution and 2) integrate into a cumulative
  /// distribution function (not normalised). Then 3) get normalisation factor.
  /// \param MaxX The maximum range of the distribution.
  explicit DistributionGenerator(double MaxX);

  /// \brief The constructor populates relevant data structures but with a
  /// custom defined bin number. Bin number defines the resolution of the
  /// distribution function.
  /// 1) calculate values for distribution and 2) integrate into a cumulative
  /// distribution function (not normalised). Then 3) get normalisation factor.
  /// \param MaxX The maximum range of the distribution.
  /// \param Bins The number of bins in the distribution. We always use the
  /// absolute value of Bins.
  explicit DistributionGenerator(double MaxX, uint32_t Bins);


  /// \brief return a random value based on the distribution function
  double getValue();

  /// \brief return the distribution value at a specific index
  double getDistValue(const double &) override;

  double MaxRange{1000.0 / 14}; // ESS 14Hz -> 71.43 ms
  uint32_t NumberOfBins{DEFAULT_BIN_COUNT};
  double BinWidth{0.0};
  double Norm{1.0};
  std::vector<double> Dist;
  std::vector<double> CDF;

private:
  // MinstdRand (fast) random number generator with Seed 1066
  std::minstd_rand gen{1066};
  // Predefined uniform real distribution between 0.0 and 1.0
  std::uniform_real_distribution<> dis{0.0, 1.0};
};

// GCOVR_EXCL_STOP
