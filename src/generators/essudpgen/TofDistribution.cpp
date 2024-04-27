// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Creates a custom TOF distribution
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <generators/essudpgen/TofDistribution.h>

///\todo move to math, could be optimised, should not use pow, can
// precalculate sqrt(2.0 * M_PI).
double gaussianPDF(double x, double mu, double sigma) {
	return 1.0 / (sigma * sqrt(2.0 * M_PI)) * exp(-(pow((x - mu)/sigma, 2)/2.0));
}


/// \brief generate Dist and CDF for the specified shape
TofDistribution::TofDistribution() {
  Dist.reserve(ArraySize);
  CDF.reserve(ArraySize);

  for (int i = 0; i < ArraySize; i++) {
    double t = i * MaxTofMs/(ArraySize-1);
    Dist[i] = 0.001 + gaussianPDF(t, 30.0, 4) + 0.6 * gaussianPDF(t, 42.0, 4);
    if (i != 0) {
      CDF[i] = CDF[i-1] + Dist[i];
    }
  }
  Norm = CDF[ArraySize-1];
}

/// \brief draw a random TOF according to distribution
// can also be improved by precalculation of bin width.
double TofDistribution::getRandomTof() {
  double Value =  dis(gen) * Norm;
  for (int i = 0; i < ArraySize; i++) {
    if (CDF[i] >= Value) {
      return i * MaxTofMs/(ArraySize-1);
    }
  }
  return MaxTofMs;
}


// GCOVR_EXCL_STOP
