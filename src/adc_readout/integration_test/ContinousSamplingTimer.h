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
#include <functional>

struct ContinousSamplingTimerData {
  /*asio::io_service *Service;
  UdpConnection *UdpCon;
  SampleRunGenerator SampleGen;

  double Settings_offset;
  double Settings_amplitude;
  double Settings_rate;
*/
  SamplingTimerData TimerData;

  int NrOfOriginalSamples;
};

class ContinousSamplingTimer : public SamplingTimer {
public:
  ContinousSamplingTimer(ContinousSamplingTimerData &data);
  void start() override;
  void stop() override;

  std::chrono::duration<size_t, std::nano> calcDelaTime();

  ContinousSamplingTimerData Data;

  void genSamplesAndQueueSend(const TimeStamp &Time);

private:
  // void handleEventTimer(const asio::error_code &Error);

  std::chrono::system_clock::time_point NextSampleTime;
  // const int NrOfOriginalSamples;
  std::chrono::system_clock::duration TimeStep;
  std::chrono::duration<size_t, std::nano> TimeStepNano;
};
