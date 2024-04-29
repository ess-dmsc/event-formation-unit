// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Creates a custom TOF distribution
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <generators/functiongenerators/CustomDistribution.h>

///\todo move to math, could be optimised, should not use pow, can
// precalculate sqrt(2.0 * M_PI).
double gaussianPDF(double x, double mu, double sigma) {
	return 1.0 / (sigma * sqrt(2.0 * M_PI)) * exp(-(pow((x - mu)/sigma, 2)/2.0));
}


/// \brief generate Dist and CDF for the specified shape
CustomDistribution::CustomDistribution(float MaxX) : MaxXVal(MaxX) {
	// Nothing to do here at the moment
	initialise();
}

void CustomDistribution::initialise() {
	Dist.reserve(Bins);
  CDF.reserve(Bins);
	BinWidth = MaxXVal/(Bins - 1);

  for (int i = 0; i < Bins; i++) {
    double t = i * BinWidth;
    Dist[i] = 0.001 + gaussianPDF(t, 30.0, 4) + 0.6 * gaussianPDF(t, 42.0, 4);
    if (i != 0) {
      CDF[i] = CDF[i-1] + Dist[i];
    }
  }
  Norm = CDF[Bins - 1];
}

/// \brief draw a random x-value according to distribution
double CustomDistribution::getRandomX() {
  double Value =  dis(gen) * Norm;
  for (int i = 0; i < Bins; i++) {
    if (CDF[i] >= Value) {
      return i * BinWidth;
    }
  }
  return MaxXVal;
}

// GCOVR_EXCL_STOP
