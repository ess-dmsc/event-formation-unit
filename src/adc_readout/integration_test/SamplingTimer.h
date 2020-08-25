/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include "../AdcTimeStamp.h"

#include "SampleRunGenerator.h"
#include "UdpConnection.h"

#include <chrono>
#include <map>
#include <random>

enum class SamplerType { PoissonDelay, AmpEventDelay, Continous };

struct SamplingTimerData {
  SamplerType Type;
  UdpConnection *UdpCon;
  SampleRunGenerator SampleGen;

  double Offset;
  double Amplitude;
  double Rate;
};

//-----------------------------------------------------------------------------

struct PoissonDelayData {
  SamplingTimerData TimerData;
  std::default_random_engine RandomGenerator;
  std::exponential_distribution<double> RandomDistribution;
};

class PoissonDelay {
public:
  PoissonDelayData Data;

  static PoissonDelay Create(UdpConnection *UdpCon, int BoxNr, int ChNr,
                             std::map<std::string, double> Settings);
  TimeDurationNano calcDelaTime();
  void genSamplesAndQueueSend(const TimeStamp &Time);
};

//-----------------------------------------------------------------------------

struct AmpEventDelayData {
  PoissonDelayData PoissonData;
  SampleRunGenerator AnodeGen;
  SampleRunGenerator XPosGen;
  SampleRunGenerator YPosGen;
};

class AmpEventDelay {
public:
  AmpEventDelayData Data;

  static AmpEventDelay Create(UdpConnection *UdpCon, int BoxNr,
                              double EventRate);
  TimeDurationNano calcDelaTime();
  void genSamplesAndQueueSend(const TimeStamp &Time);
};

//-----------------------------------------------------------------------------

struct ContinousSamplingTimerData {
  SamplingTimerData TimerData;
  TimeDurationNano TimeStepNano;
};

class ContinousSamplingTimer {
public:
  ContinousSamplingTimerData Data;

  static ContinousSamplingTimer Create(UdpConnection *UdpCon, int BoxNr,
                                       int ChNr,
                                       std::map<std::string, double> Settings);
  TimeDurationNano calcDelaTime();
  void genSamplesAndQueueSend(const TimeStamp &Time);
};
