// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Creates a custom TOF distribution
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <random>

class TofDistribution {
public:

  /// \brief
  TofDistribution();

  /// \brief
  double getRandomTof();

  float MaxTofMs = 1000.0/14; // ESS 14Hz -> 71.43 ms
  int Bins{512};
  float BinWidth{0.0};
  float Norm{1};
  std::vector<float> Dist;
  std::vector<float> CDF;


  std::mt19937 gen{1066}; // Standard mersenne_twister_engine seeded with 1066
  std::uniform_real_distribution<> dis{0.0, 1.0};
};
// GCOVR_EXCL_STOP
