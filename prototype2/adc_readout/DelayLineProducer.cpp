/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Simple peak finding implementation.
 */

#include "DelayLineProducer.h"
#include <chrono>
#include <cmath>
#include <limits>

using namespace std::chrono_literals;

DelayLineProducer::DelayLineProducer(std::string Broker, std::string Topic,
                                     AdcSettings EfuSettings)
    : Producer(std::move(Broker), std::move(Topic)),
      Settings(std::move(EfuSettings)),
      Serializer("delay_line_detector", 200, 200ms, this,
                 EventSerializer::TimestampMode::INDEPENDENT_EVENTS) {
  PulseProcessingThread =
      std::thread(&DelayLineProducer::pulseProcessingFunction, this);
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
        serializeAndSendEvent(Event);
      }
      NewPulse = false;
    } else {
      std::this_thread::sleep_for(5ms);
    }
  }
}

void DelayLineProducer::serializeAndSendEvent(const DelayLineEvent &Event) {
  std::uint32_t EventPosition = essgeometry.pixel2D(
      static_cast<uint32_t>(Event.X), static_cast<uint32_t>(Event.Y));
  ++EventCounter;
  Serializer.addEvent(std::unique_ptr<EventData>(
      new EventData{Event.Timestamp, EventPosition,
                    static_cast<std::uint32_t>(Event.Amplitude), 0, 0, 0, 0}));
}

DelayLineProducer::~DelayLineProducer() {
  RunThread.store(false);
  if (PulseProcessingThread.joinable()) {
    PulseProcessingThread.join();
  }
}

void DelayLineProducer::addReferenceTimestamp(
    RawTimeStamp const &ReferenceTimestamp) {
  std::uint64_t TempRefTimeStamp = ReferenceTimestamp.getTimeStampNS();
  if (CurrentReferenceTimestamp != TempRefTimeStamp) {
    CurrentReferenceTimestamp = TempRefTimeStamp;
    Serializer.addReferenceTimestamp(CurrentReferenceTimestamp);
  }
}
