/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Simple peak finding implementation.
 */

#include "DelayLineProducer.h"
#include "DelayLineEventFormation.h"
#include "ev42_events_generated.h"
#include <chrono>
#include <cmath>
#include <limits>

using namespace std::chrono_literals;

DelayLineProducer::DelayLineProducer(std::string broker, std::string topicstr,
                                     AdcSettings EfuSettings)
    : Producer(broker, topicstr), Settings(EfuSettings) {
  PulseProcessingThread =
      std::thread(&DelayLineProducer::PulseProcessingThread, this);
}

void DelayLineProducer::addPulse(PulseParameters const Pulse) {
  std::lock_guard<std::mutex> PulseLock(PulseMutex);
  PulseQueue.emplace(Pulse);
}

void DelayLineProducer::pulseProcessingFunction() {
  DelayLineEventFormation Efu(Settings);
  PulseParameters TempPulse;
  bool NewPulse = false;
  while (RunThread) {
    {
      std::lock_guard<std::mutex> PulseLock(PulseMutex);
      if (not PulseQueue.empty()) {
        TempPulse = PulseQueue.front();
        PulseQueue.pop();
        NewPulse = true;
      }
    }
    if (NewPulse) {
      Efu.addPulse(TempPulse);
      if (Efu.hasValidEvent()) {
        auto Event = Efu.popEvent();
        serializeEvent(Event);
        // ProducerBase::produce(<#void *buffer#>, <#size_t bytes#>)
      }
      NewPulse = false;
    } else {
      std::this_thread::sleep_for(5ms);
    }
  }
}

DelayLineProducer::~DelayLineProducer() {
  RunThread = false;
  if (PulseProcessingThread.joinable()) {
    PulseProcessingThread.join();
  }
}
