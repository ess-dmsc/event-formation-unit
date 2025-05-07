// Copyright (C) 2024 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief platform independent equivalent replacement wrapper for the cheap
/// and fast time stamp counter (TSC). For further details regarding the TSC
/// timer, see https://tinyurl.com/mr242w8c.
///
/// Uses the standard chrono library to provide a high resolution timer.
//===----------------------------------------------------------------------===//

#pragma once

#include <chrono>
#include <cstdint>

class Timer {

public:
  /// \brief Create a check timer with a timeout value in nanoseconds
  /// \param TimeOut If a time out is not set, use this 'infinite' value
  Timer(uint64_t TimeOut=0xffffffffffffffff);

  /// \brief If a timeout has occurred, then reset timer
  bool timeout();

  /// \brief Record current time point
  void reset();

  /// \brief Return time in nano seconds since last time point
  uint64_t timeNS();

  /// \return Return time in micro seconds since last time point
  uint64_t timeUS();

  /// \return Return time in milli seconds since last time point
  uint64_t timeMS();

private:
std::chrono::time_point<std::chrono::high_resolution_clock,
                        std::chrono::nanoseconds> T0;

uint64_t TimeOutNS;
};
