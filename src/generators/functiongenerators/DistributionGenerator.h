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

class DistributionGenerator {
public:

  ///\brief The constructor populates relevant data structures
  /// 1) calculate values for distribution and 2) integrate into a cumulative
  /// distribution function (not normalised). Then 3) get normalisation factor.
  DistributionGenerator(float MaxX);

  ///\brief return a random value based on the distribution function
  double getValue();

public:
  float MaxRange{1000.0/14}; // ESS 14Hz -> 71.43 ms
  int Bins{512};
  float BinWidth{0.0};
  float Norm{1.0};
  std::vector<float> Dist;
  std::vector<float> CDF;

  // objects for random number generation
  std::mt19937 gen{1066}; // Standard mersenne_twister_engine. Seed 1066
  std::uniform_real_distribution<> dis{0.0, 1.0};
};
// GCOVR_EXCL_STOP
