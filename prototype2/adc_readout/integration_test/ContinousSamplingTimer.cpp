/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include "ContinousSamplingTimer.h"
#include <ciso646>

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

ContinousSamplingTimer::ContinousSamplingTimer(
    std::function<void(RawTimeStamp const &)> OnTimer,
    asio::io_service &AsioService, int NrOfSamples, int OversamplingFactor)
    : SamplingTimer(std::move(OnTimer)), SampleTimer(AsioService),
      NextSampleTime(system_clock::now()),
      NrOfOriginalSamples(NrOfSamples * OversamplingFactor) {
  auto Temp = duration<std::uint64_t, std::nano>(
      static_cast<std::uint64_t>((NrOfOriginalSamples / TimeFracMax) * 1e9));
  TimeStep = duration_cast<system_clock::duration>(Temp);
}

void ContinousSamplingTimer::start() {
  NextSampleTime += TimeStep;
  SampleTimer.expires_at(NextSampleTime);
  auto WaitHandlerGlue = [this](auto &Error) { this->handleEventTimer(Error); };
  SampleTimer.async_wait(WaitHandlerGlue);
}

void ContinousSamplingTimer::stop() { SampleTimer.cancel(); }

void ContinousSamplingTimer::handleEventTimer(const asio::error_code &Error) {
  if (not Error) {
    runFunction();
    start();
  }
}
