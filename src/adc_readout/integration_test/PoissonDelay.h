/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include "../AdcTimeStamp.h"
#include "SamplingTimer.h"

#include <functional>
#include <random>

#include "SampleRunGenerator.h"
#include "UdpConnection.h"

struct PoissonDelayData {
  SamplingTimerData TimerData;
};

class PoissonDelay : public SamplingTimer {
public:
  PoissonDelay(PoissonDelayData &data);

  void start() override;
  void stop() override;

  std::chrono::duration<size_t, std::nano> calcDelaTime();

  PoissonDelayData data;

  void genSamplesAndQueueSend(const TimeStamp &Time);

  std::random_device RandomDevice;
  std::default_random_engine RandomGenerator;
  std::exponential_distribution<double> RandomDistribution;
};
