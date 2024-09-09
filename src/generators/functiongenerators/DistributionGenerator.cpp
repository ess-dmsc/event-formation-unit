// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Creates a custom distribution function
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <cstdint>
#include <generators/functiongenerators/DistributionGenerator.h>

///\todo could be optimised, but this is not the bottleneck
static double gaussianPDF(double X, double Mu, double Sigma) {
  return 1.0 / (Sigma * sqrt(2.0 * M_PI)) *
         exp(-(pow((X - Mu) / Sigma, 2) / 2.0));
}

DistributionGenerator::DistributionGenerator(double MaxVal)
    : DistributionGenerator(MaxVal, 512) {}

/// \brief generate Dist and CDF for the specified shape. Always use the absolute value of Bins.
DistributionGenerator::DistributionGenerator(double MaxVal, int Bins) : MaxRange(MaxVal), NumberOfBins(abs(Bins)) {
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

double DistributionGenerator::getValue(const double &Pos) {
  int Index = static_cast<int>(Pos / BinWidth);
  return Dist[Index];
}

///
/// \brief draw a random value according to distribution
/// \todo this is the slow part of this implementation. Optimisation ideas:
/// precalculate an array of values and then reuse these rather than calculating
/// values every time.
/// Or implement a faster search than the loop below (dictionary or something)
double DistributionGenerator::getValue() {
  double Value = dis(gen) * Norm;
  for (uint i = 0; i < NumberOfBins; i++) {
    if (CDF[i] >= Value) {
      return i * BinWidth;
    }
  }
  return MaxRange;
}

// GCOVR_EXCL_STOP
