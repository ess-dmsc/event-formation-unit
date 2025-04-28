// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief wrapper for slow but accurate time
//===----------------------------------------------------------------------===//

#pragma once
#include <chrono>
#include <cstdint>

class Timer {
  using HRClock = std::chrono::high_resolution_clock;
  using TP = std::chrono::_V2::system_clock::time_point;

public:
  /// \brief Constructor
  Timer();

  /// \brief Record current time point
  void reset();

  /// \return Return time in micro seconds since last time point
  uint64_t timeus();

  /// \return Return time in milli seconds since last time point
  uint64_t timems();

  /// \return the current time
  static TP now() {
    return HRClock::now();
  }

private:
  TP T0;
};
