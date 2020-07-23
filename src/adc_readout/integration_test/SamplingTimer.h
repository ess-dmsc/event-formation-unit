/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include "../AdcTimeStamp.h"
#include <functional>

#ifndef assert
#define assert(...) /**/
#endif
#include <asio.hpp>
#include "SampleRunGenerator.h"
#include "UdpConnection.h"

enum class SamplerType{
  PoissonDelay,
  AmpEventDelay,
  Continous
};

struct SamplingTimerData {
  SamplerType Type;
  asio::io_service *Service;
  UdpConnection *UdpCon;
  SampleRunGenerator SampleGen;

  double Settings_offset;
  double Settings_amplitude;
  double Settings_rate;
};

class SamplingTimer {
public:
  explicit SamplingTimer(std::function<void(TimeStamp const &)> OnTimer);
  virtual ~SamplingTimer() = default;
  virtual void start() = 0;
  virtual void stop() = 0;

  //virtual void genSamplesAndQueueSend();

  std::function<void(TimeStamp const &)> SamplingFunc;
};