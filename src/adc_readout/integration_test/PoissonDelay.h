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
  std::default_random_engine RandomGenerator;
  std::exponential_distribution<double> RandomDistribution;
};

class PoissonDelay {
public:
  PoissonDelayData data;

  std::chrono::duration<size_t, std::nano> calcDelaTime() {
    auto DelayTime = data.RandomDistribution(data.RandomGenerator);
    auto NextEventDelay = std::chrono::duration<size_t, std::nano>(
        static_cast<size_t>(DelayTime * 1e9));
    return NextEventDelay;
  }

  void genSamplesAndQueueSend(const TimeStamp &Time) {
    std::pair<void *, std::size_t> SampleRun =
        data.TimerData.SampleGen.generate(data.TimerData.Settings_amplitude,
                                          Time);
    data.TimerData.UdpCon->addSamplingRun(SampleRun.first, SampleRun.second,
                                          Time);
  }
};
