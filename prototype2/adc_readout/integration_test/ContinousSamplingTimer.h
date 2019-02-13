/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include "../AdcTimeStamp.h"
#include "SamplingTimer.h"
#include <asio.hpp>
#include <chrono>
#include <functional>

class ContinousSamplingTimer : public SamplingTimer {
public:
  ContinousSamplingTimer(std::function<void(RawTimeStamp const &)> OnTimer,
                         asio::io_service &AsioService, int NrOfSamples,
                         int OversamplingFactor);
  void start() override;
  void stop() override;

private:
  void handleEventTimer(const asio::error_code &Error);
  asio::system_timer SampleTimer;

  std::chrono::system_clock::time_point NextSampleTime;
  const int NrOfOriginalSamples;
  const double TimeFracMax = 88052500.0 / 2;
  std::chrono::system_clock::duration TimeStep;
};
