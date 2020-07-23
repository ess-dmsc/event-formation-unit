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
    : SamplingTimer([](TimeStamp const &) {}), Data(data) {

  NextSampleTime = system_clock::now();

  const double TimeFracMax = 88052500.0 / 2;

  TimeStepNano =
      std::chrono::duration<size_t, std::nano>(static_cast<std::uint64_t>(
          (data.NrOfOriginalSamples / TimeFracMax) * 1e9));

  TimeStep = duration_cast<system_clock::duration>(TimeStepNano);
}

std::chrono::duration<size_t, std::nano>
ContinousSamplingTimer::calcDelaTime() {
  return TimeStepNano;
}

void ContinousSamplingTimer::start() {}

void ContinousSamplingTimer::genSamplesAndQueueSend(const TimeStamp &Time) {
  std::pair<void *, std::size_t> SampleRun = Data.TimerData.SampleGen.generate(
      Data.TimerData.Settings_amplitude, Time);
  Data.TimerData.UdpCon->addSamplingRun(SampleRun.first, SampleRun.second,
                                        Time);
}

void ContinousSamplingTimer::stop() {}
