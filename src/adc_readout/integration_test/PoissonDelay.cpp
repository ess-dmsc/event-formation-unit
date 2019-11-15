/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include "PoissonDelay.h"
#include <ciso646>
#include <iostream>

PoissonDelay::PoissonDelay(std::function<void(TimeStamp const &)> OnEvent,
                           asio::io_service &AsioService, double EventRate)
    : SamplingTimer(std::move(OnEvent)), EventTimer(AsioService),
      RandomGenerator(RD()), RandomDistribution(EventRate) {}

void PoissonDelay::start() {
  auto DelayTime = RandomDistribution(RandomGenerator);
  auto NextEventDelay =
      std::chrono::duration<int, std::micro>(static_cast<int>(DelayTime * 1e6));
  EventTimer.expires_after(NextEventDelay);
  auto WaitHandlerGlue = [this](auto &Error) { this->handleEventTimer(Error); };
  EventTimer.async_wait(WaitHandlerGlue);
}

void PoissonDelay::stop() { EventTimer.cancel(); }

void PoissonDelay::handleEventTimer(const asio::error_code &Error) {
  if (not Error) {
    runFunction();
    start();
  }
}
