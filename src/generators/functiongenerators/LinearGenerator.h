// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generates random data with a weighted distribution function
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <generators/functiongenerators/FunctionGenerator.h>
#include <random>

class LinearGenerator : public FunctionGenerator {
public:
  LinearGenerator(double gradient, uint32_t offset = 0.0)
      : Gradient(gradient), Offset(offset){};

  ///\brief return the distribution value at a specific index
  double getValue(const double &Pos) override {
    double value = Offset;

    for (int i = 0; i <= Pos; i++) {
      value += Gradient;
    }

    return value;
  }

public:
  double Gradient;
  uint32_t Offset;
};

// GCOVR_EXCL_STOP
