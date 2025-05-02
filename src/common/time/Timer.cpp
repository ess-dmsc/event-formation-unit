// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation - just calls chrono functions
//===----------------------------------------------------------------------===//

#include <common/time/Timer.h>

Timer::Timer() {
  T0 = HRClock::now();
}

void Timer::reset() {
  T0 = HRClock::now();
}

uint64_t Timer::timeUS() {
  TP T1 = HRClock::now();

  return std::chrono::duration_cast<std::chrono::microseconds>(T1 - T0).count();
}

uint64_t Timer::timeMS() {
  TP T1 = HRClock::now();

  return std::chrono::duration_cast<std::chrono::milliseconds>(T1 - T0).count();
}
