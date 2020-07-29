/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include "../AdcTimeStamp.h"
#include "SampleRunGenerator.h"
#include "SamplingTimer.h"
#include "UdpConnection.h"
#include <chrono>

struct ContinousSamplingTimerData {
  SamplingTimerData TimerData;
  std::chrono::duration<size_t, std::nano> TimeStepNano;
};

class ContinousSamplingTimer {
public:
  ContinousSamplingTimerData Data;

  std::chrono::duration<size_t, std::nano> calcDelaTime() {
    return Data.TimeStepNano;
  }

  void genSamplesAndQueueSend(const TimeStamp &Time) {
    std::pair<void *, std::size_t> SampleRun =
        Data.TimerData.SampleGen.generate(Data.TimerData.Settings_amplitude,
                                          Time);
    Data.TimerData.UdpCon->addSamplingRun(SampleRun.first, SampleRun.second,
                                          Time);
  }
};
