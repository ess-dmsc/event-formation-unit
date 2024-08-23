// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generates random data with a weighted distribution function
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <random>
#include <generators/functiongenerators/FunctionGenerator.h>

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
  /// \brief The constructor populates relevant data structures
  /// 1) calculate values for distribution and 2) integrate into a cumulative
  /// distribution function (not normalised). Then 3) get normalisation factor.
  DistributionGenerator(double MaxX);

  /// \brief return a random value based on the distribution function
  double getValue();

  /// \brief return the distribution value at a specific index
  double getValue(const double &) override;

public:
  double MaxRange{1000.0 / 14}; // ESS 14Hz -> 71.43 ms
  int Bins{512};
  double BinWidth{0.0};
  double Norm{1.0};
  std::vector<double> Dist;
  std::vector<double> CDF;

  // objects for random number generation
  std::mt19937 gen{1066}; // Standard mersenne_twister_engine. Seed 1066
  std::uniform_real_distribution<> dis{0.0, 1.0};
};
// GCOVR_EXCL_STOP
