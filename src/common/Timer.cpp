// Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation - just calls chrono functions
//===----------------------------------------------------------------------===//

#include <common/Timer.h>

/** */
Timer::Timer(void) { T0 = HRClock::now(); }

/** */
void Timer::reset(void) { T0 = HRClock::now(); }

/** */
uint64_t Timer::timeus(void) {
  Timer::TP T1 = Timer::HRClock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(T1 - T0).count();
}
