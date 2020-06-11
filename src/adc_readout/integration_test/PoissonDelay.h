/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include "../AdcTimeStamp.h"
#include "SamplingTimer.h"
#ifndef assert
#define assert(...) /**/
#endif
#include <asio.hpp>
#include <functional>
#include <random>

#include "SampleRunGenerator.h"
#include "FPGASim.h"

struct PoissonDelayData {
  asio::io_service *Service;
  FPGASim *FPGAPtr;
  SampleRunGenerator SampleGen;

  double Settings_offset;
  double Settings_amplitude;
  double Settings_rate;
};

class PoissonDelay : public SamplingTimer {
public:
  PoissonDelay(PoissonDelayData &sdg);

  void start() override;
  void stop() override;

  PoissonDelayData data;

  void genSamplesAndEnqueueSend() override;

  asio::high_resolution_timer EventTimer;
  // asio::system_timer EventTimer;
  std::random_device RandomDevice;
  std::default_random_engine RandomGenerator;
  std::exponential_distribution<double> RandomDistribution;
};
