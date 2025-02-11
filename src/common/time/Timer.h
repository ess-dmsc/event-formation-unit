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

  typedef std::chrono::high_resolution_clock HRClock;
  typedef std::chrono::time_point<HRClock> TP;

public:
  Timer(void);

  void reset(void); ///< record current time_point

  uint64_t timeus(void); ///< time since tp

private:
  TP T0;
};
