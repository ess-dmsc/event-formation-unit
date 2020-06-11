/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include "ContinousSamplingTimer.h"
#include <ciso646>
#include <cmath>

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

ContinousSamplingTimer::ContinousSamplingTimer(ContinousSamplingTimerData &data)
    : SamplingTimer([](TimeStamp const &) {}), Data(data),
      SampleTimer(*data.Service) {

  NextSampleTime = system_clock::now();

  const double TimeFracMax = 88052500.0 / 2;

  duration<std::uint64_t, std::nano> Temp(static_cast<std::uint64_t>(
      (data.NrOfOriginalSamples / TimeFracMax) * 1e9));

  TimeStep = duration_cast<system_clock::duration>(Temp);
}

void ContinousSamplingTimer::start() {
  NextSampleTime += TimeStep;
  SampleTimer.expires_at(NextSampleTime);
  SampleTimer.async_wait([this](auto &Error) {
    if (not Error) {
      genSamplesAndEnqueueSend();
      start();
    }
  });
}

void ContinousSamplingTimer::genSamplesAndEnqueueSend() {
  auto Now = system_clock::now();
  auto NowSeconds = duration_cast<seconds>(Now.time_since_epoch()).count();
  double NowSecFrac =
      (duration_cast<nanoseconds>(Now.time_since_epoch()).count() / 1e9) -
      NowSeconds;
  std::uint32_t Ticks = std::lround(NowSecFrac * (88052500 / 2.0));

  RawTimeStamp rts{static_cast<uint32_t>(NowSeconds), Ticks};
  TimeStamp Time(rts, TimeStamp::ClockMode::External);

  ////////////////

  std::pair<void *, std::size_t> SampleRun =
      Data.SampleGen.generate(Data.Settings_amplitude, Time);
  Data.UdpCon->addSamplingRun(SampleRun.first, SampleRun.second, Time);
}

void ContinousSamplingTimer::stop() { SampleTimer.cancel(); }
