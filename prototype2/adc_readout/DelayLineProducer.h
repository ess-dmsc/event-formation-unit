/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Simple peak finding algorithm implementation header.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "AdcSettings.h"
#include "AdcTimeStamp.h"
#include "DelayLineEventFormation.h"
#include "PulseParameters.h"
#include <common/Producer.h>
#include <queue>
#include <thread>

class DelayLineProducer : public Producer {
public:
  DelayLineProducer(std::string broker, std::string topicstr,
                    AdcSettings EfuSettings);
  ~DelayLineProducer();
  void addPulse(PulseParameters const Pulse);

protected:
  void serializeEvent(DelayLineEvent const &){};
  void pulseProcessingFunction();
  using Producer::produce;
  AdcSettings Settings;
  std::thread PulseProcessingThread;
  std::mutex PulseMutex;
  std::queue<PulseParameters> PulseQueue;
  std::atomic_bool RunThread{true};
};
