// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Creates a custom distribution function
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START
#include <generators/functiongenerators/DistributionGenerator.h>

#include <algorithm>
#include <cstdint>

///\todo could be optimised, but this is not the bottleneck
static double gaussianPDF(double X, double Mu, double Sigma) {
  return 1.0 / (Sigma * sqrt(2.0 * M_PI)) *
         exp(-(pow((X - Mu) / Sigma, 2) / 2.0));
}

DistributionGenerator::DistributionGenerator(uint16_t Frequency)
: DistributionGenerator(1000.0 / Frequency, DEFAULT_BIN_COUNT) {}

DistributionGenerator::DistributionGenerator(uint16_t Frequency, uint32_t Bins)
: DistributionGenerator(1000.0 / Frequency, Bins) {}

DistributionGenerator::DistributionGenerator(double MaxVal)
    : DistributionGenerator(MaxVal, DEFAULT_BIN_COUNT) {}

/// \brief generate Dist and CDF for the specified shape. Always use the absolute value of Bins.
DistributionGenerator::DistributionGenerator(double MaxVal, uint32_t Bins) : MaxRange(MaxVal), NumberOfBins(Bins) {
  Dist.resize(NumberOfBins);
  CDF.resize(NumberOfBins);
  BinWidth = MaxRange / (NumberOfBins - 1);

  for (uint i = 0; i < NumberOfBins; i++) {
    double XCoord = i * BinWidth;
    Dist[i] = 0.001 + gaussianPDF(XCoord, 30.0, 4) +
              0.6 * gaussianPDF(XCoord, 42.0, 4);
    if (i != 0) {
      CDF[i] = CDF[i - 1] + Dist[i];
    }
  }
  Norm = CDF[NumberOfBins - 1];
}

double DistributionGenerator::getValueByIndex(double Pos) {
  int Index = static_cast<int>(Pos / BinWidth);
  return Dist[Index];
}

///
/// \brief draw a random value according to the distribution
double DistributionGenerator::getValue() {
  // Find the index of this value
  const double Value = dis(gen) * Norm;

  // Since CDF is sorted and increasing, we can use std::upper_bound
  const auto it = std::upper_bound(CDF.cbegin(), CDF.cend(), Value);
  if (it == CDF.end()) {
    return MaxRange;
  }
  const size_t index = std::distance(CDF.cbegin(), it);

  return index * BinWidth;
}

// GCOVR_EXCL_STOP
