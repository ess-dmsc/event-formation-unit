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

#include <random>
#include <chrono>
#include <map>

enum class SamplerType { PoissonDelay, AmpEventDelay, Continous };

struct SamplingTimerData {
  SamplerType Type;
  UdpConnection *UdpCon;
  SampleRunGenerator SampleGen;

  double Settings_offset;
  double Settings_amplitude;
  double Settings_rate;
};

//-----------------------------------------------------------------------------

struct PoissonDelayData {
  SamplingTimerData TimerData;
  std::default_random_engine RandomGenerator;
  std::exponential_distribution<double> RandomDistribution;
};

class PoissonDelay {
public:
  PoissonDelayData data;

  static PoissonDelay Create(UdpConnection *UdpCon, int BoxNr, int ChNr,
                             std::map<std::string, double> Settings);
  std::chrono::duration<size_t, std::nano> calcDelaTime();
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
  AmpEventDelayData data;

  static AmpEventDelay Create(UdpConnection *UdpCon, int BoxNr,
                              double EventRate);
  std::chrono::duration<size_t, std::nano> calcDelaTime();
  void genSamplesAndQueueSend(const TimeStamp &Time);
};

//-----------------------------------------------------------------------------

struct ContinousSamplingTimerData {
  SamplingTimerData TimerData;
  std::chrono::duration<size_t, std::nano> TimeStepNano;
};

class ContinousSamplingTimer {
public:
  ContinousSamplingTimerData Data;

  static ContinousSamplingTimer Create(UdpConnection *UdpCon, int BoxNr,
                                       int ChNr,
                                       std::map<std::string, double> Settings);
  std::chrono::duration<size_t, std::nano> calcDelaTime();
  void genSamplesAndQueueSend(const TimeStamp &Time);
};