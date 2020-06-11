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
#ifndef assert
#define assert(...) /**/
#endif
#include <asio.hpp>
#include <chrono>
#include <functional>

struct ContinousSamplingTimerData {
  asio::io_service *Service;
  UdpConnection *UdpCon;
  SampleRunGenerator SampleGen;
  int NrOfOriginalSamples;

  double Settings_offset;
  double Settings_amplitude;
  double Settings_rate;
};

class ContinousSamplingTimer : public SamplingTimer {
public:
  ContinousSamplingTimer(ContinousSamplingTimerData &data);
  void start() override;
  void stop() override;

  ContinousSamplingTimerData Data;

  void genSamplesAndEnqueueSend() override;

private:
  void handleEventTimer(const asio::error_code &Error);
  asio::system_timer SampleTimer;

  std::chrono::system_clock::time_point NextSampleTime;
  // const int NrOfOriginalSamples;
  std::chrono::system_clock::duration TimeStep;
};
