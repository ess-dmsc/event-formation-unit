// Copyright (C) 2024 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of the check timer class which uses the standard chrono
/// library to provide a high resolution timer.
//===----------------------------------------------------------------------===//

#include <common/time/CheckTimer.h>

CheckTimer::CheckTimer(uint64_t Timeout)
  : TimeOutNS(Timeout) {
  reset();
}

bool CheckTimer::timeout() {
  if (timeNS() >= TimeOutNS) {
    reset();
    return true;
  }

  return false;
}

void CheckTimer::reset() {
  T0 = std::chrono::high_resolution_clock::now();
}

uint64_t CheckTimer::timeNS() {
  auto T1 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(T1 - T0).count();
}
