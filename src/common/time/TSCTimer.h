// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief wrapper for the cheap and fast time stamp counter (TSC)
///
/// TSC is a 64 bit counter running at CPU clock. Can be used (with caution)
/// as a high resolution timer. A detailed description of the fundamentals
/// behind the implementation, can be found here https://tinyurl.com/mr242w8c
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>

class TSCTimer {

public:
  /// \brief Create a TSC timer with a timeout value
  /// \param TimeOut If a time out is not set, use this 'infinite' value
  TSCTimer(uint64_t TimeOut=0xffffffffffffffff);

  /// \brief If a time out occurred, then reset timer
  bool timeout();

  /// \brief Record current time_point
  void reset();

  /// \brief Return time since last
  uint64_t timeTSC();

private:
  /// Reference TSC time point
  uint64_t T0;

  // If timeout is not set, then use 'infinite' value
  uint64_t TimeOutTicks;
};
