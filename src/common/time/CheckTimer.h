// Copyright (C) 2024 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief platform independent equivalent replacement of wrapper for the cheap
/// and fast time stamp counter (TSC)
///
/// Uses the standard chrono library to provide a high resolution timer.
//===----------------------------------------------------------------------===//

#pragma once

#include <chrono>
#include <cstdint>

class CheckTimer {

public:
  /// \brief Create a check timer with a timeout value in nanoseconds
  /// \param TimeOut If a time out is not set, use this 'infinite' value
  CheckTimer(uint64_t TimeOut=0xffffffffffffffff);

  /// \brief If a timeout has occurred, then reset timer
  bool timeout();

  /// \brief Record current time point
  void reset();

  /// \brief Return time since last recorded time stamp
  uint64_t timeNS();

private:
  std::chrono::high_resolution_clock::time_point T0;
  uint64_t TimeOutNS;
};
