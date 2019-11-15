/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include "SamplingTimer.h"
#include <chrono>
#include <ciso646>
#include <cmath>

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

SamplingTimer::SamplingTimer(std::function<void(TimeStamp const &)> OnTimer)
    : SamplingFunc(std::move(OnTimer)) {}

void SamplingTimer::runFunction() {
  auto Now = system_clock::now();
  auto NowSeconds = duration_cast<seconds>(Now.time_since_epoch()).count();
  double NowSecFrac =
      (duration_cast<nanoseconds>(Now.time_since_epoch()).count() / 1e9) -
      NowSeconds;
  std::uint32_t Ticks = std::lround(NowSecFrac * (88052500 / 2.0));
  SamplingFunc({{static_cast<uint32_t>(NowSeconds), Ticks}, TimeStamp::ClockMode::External});
}
