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

PoissonDelay::PoissonDelay(PoissonDelayData &sdg)
    : SamplingTimer([](TimeStamp const &Time) {
        std::cout << "bad PoissonDelay::PoissonDelay bind" << Time.getSeconds();
      }),
      data(sdg), EventTimer(*data.Service), RandomGenerator(RandomDevice()),
      RandomDistribution(data.Settings_rate) {}

void PoissonDelay::start() {
  auto DelayTime = RandomDistribution(RandomGenerator);
  auto NextEventDelay = std::chrono::duration<size_t, std::nano>(
      static_cast<size_t>(DelayTime /** 1e6*/));
  EventTimer.expires_after(NextEventDelay);

  EventTimer.async_wait([this](auto &Error) {
    if (not Error) {
      runFunction();
      start();
    }
  });
}

void PoissonDelay::runFunction() {
  auto Now = system_clock::now();
  auto NowSeconds = duration_cast<seconds>(Now.time_since_epoch()).count();
  double NowSecFrac =
      (duration_cast<nanoseconds>(Now.time_since_epoch()).count() / 1e9) -
      NowSeconds;
  std::uint32_t Ticks = std::lround(NowSecFrac * (88052500 / 2.0));

  RawTimeStamp rts{static_cast<uint32_t>(NowSeconds), Ticks};
  TimeStamp Time(rts, TimeStamp::ClockMode::External);

  ////////////////
  std::pair<void *, std::size_t> SampleRun =
      data.SampleGen.generate(data.Settings_amplitude, Time);
  data.FPGAPtr->addSamplingRun(SampleRun.first, SampleRun.second, Time);
}

void PoissonDelay::stop() { EventTimer.cancel(); }