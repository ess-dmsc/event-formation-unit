/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Simple peak finding implementation.
 */

#include "DelayLineProducer.h"
#include "ev42_events_generated.h"
#include <chrono>
#include <cmath>
#include <limits>

using namespace std::chrono_literals;

DelayLineProducer::DelayLineProducer(std::string Broker, std::string Topic,
                                     AdcSettings EfuSettings)
    : Producer(std::move(Broker), std::move(Topic)),
      Settings(std::move(EfuSettings)) {
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
  flatbuffers::FlatBufferBuilder Builder;
  auto SourceName = Builder.CreateString("delay_line_detector");
  auto ToF_Vector = Builder.CreateVector(
      std::vector<std::uint32_t>{static_cast<unsigned int>(Event.Amplitude)});
  std::uint32_t EventPosition = essgeometry.pixel2D(
      static_cast<uint32_t>(Event.X), static_cast<uint32_t>(Event.Y));
  if (EventPosition == 0) {
    // \todo report geometry error to grafana and return?
  }
  auto DetectorID_Vector =
      Builder.CreateVector(std::vector<std::uint32_t>{EventPosition});
  EventMessageBuilder EvBuilder(Builder);
  EvBuilder.add_source_name(SourceName);
  EvBuilder.add_message_id(EventCounter++);
  EvBuilder.add_pulse_time(Event.Timestamp);
  EvBuilder.add_time_of_flight(ToF_Vector);
  EvBuilder.add_detector_id(DetectorID_Vector);
  Builder.Finish(EvBuilder.Finish(), EventMessageIdentifier());
  produce(reinterpret_cast<char *>(Builder.GetBufferPointer()),
          Builder.GetSize());
}

DelayLineProducer::~DelayLineProducer() {
  RunThread = false;
  if (PulseProcessingThread.joinable()) {
    PulseProcessingThread.join();
  }
}
