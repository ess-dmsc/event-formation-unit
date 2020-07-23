/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include "PoissonDelay.h"
#include <ciso646>
#include <iostream>

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

PoissonDelay::PoissonDelay(PoissonDelayData &data)
    : SamplingTimer([](TimeStamp const &Time) {
        std::cout << "bad PoissonDelay::PoissonDelay bind" << Time.getSeconds();
      }),
      data(data), RandomGenerator(RandomDevice()),
      RandomDistribution(data.TimerData.Settings_rate) {}

std::chrono::duration<size_t, std::nano> PoissonDelay::calcDelaTime() {
  auto DelayTime = RandomDistribution(RandomGenerator);
  auto NextEventDelay = std::chrono::duration<size_t, std::nano>(
      static_cast<size_t>(DelayTime /** 1e6*/));
  return NextEventDelay;
}

void PoissonDelay::start() {}

void PoissonDelay::genSamplesAndQueueSend(const TimeStamp &Time) {
  std::pair<void *, std::size_t> SampleRun = data.TimerData.SampleGen.generate(
      data.TimerData.Settings_amplitude, Time);
  data.TimerData.UdpCon->addSamplingRun(SampleRun.first, SampleRun.second,
                                        Time);
}

void PoissonDelay::stop() {}