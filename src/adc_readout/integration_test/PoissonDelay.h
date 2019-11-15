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
#include <functional>
#include <random>

class PoissonDelay : public SamplingTimer {
public:
  PoissonDelay(std::function<void(TimeStamp const &)> OnEvent,
               asio::io_service &AsioService, double EventRate);
  void start() override;
  void stop() override;

private:
  void handleEventTimer(const asio::error_code &Error);
  asio::system_timer EventTimer;
  std::random_device RD;
  std::default_random_engine RandomGenerator;
  std::exponential_distribution<double> RandomDistribution;
};
