// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Serialize neutron events by putting them into a flatbuffer.
///
//===----------------------------------------------------------------------===//
#include "EventSerializer.h"
#include "EventBuffer.h"
#include <dtdb_adc_pulse_debug_generated.h>
#include <ev42_events_generated.h>
#include <flatbuffers/flatbuffers.h>
#include <limits>
#include <vector>

EventSerializer::EventSerializer(std::string SourceName, size_t BufferSize,
                                 std::chrono::milliseconds TransmitTimeout,
                                 ProducerBase *KafkaProducer,
                                 TimestampMode Mode, OffsetTime UseOffset)
    : Name(std::move(SourceName)), Timeout(TransmitTimeout),
      EventBufferSize(BufferSize), Producer(KafkaProducer), CMode(Mode),
      RefTimeOffset(UseOffset) {
  SerializeThread = std::thread(&EventSerializer::serialiseThreadFunc, this);
}

EventSerializer::~EventSerializer() {
  RunThread.store(false);
  if (SerializeThread.joinable()) {
    SerializeThread.join();
  }
}

void EventSerializer::addEvent(std::unique_ptr<EventData> Event) {
  EventQueue.enqueue(std::move(Event));
}

void EventSerializer::addReferenceTimestamp(std::uint64_t const Timestamp) {
  if (CMode != TimestampMode::TIME_REFERENCED) {
    return;
  }
  ReferenceTimeQueue.enqueue(Timestamp);
}

struct FBVector {
  FBVector(flatbuffers::FlatBufferBuilder &Builder, size_t MaxEvents) {
    Offset = Builder.CreateUninitializedVector(MaxEvents, sizeof(std::uint32_t),
                                               &DataPtr);
    Data = nonstd::span<std::uint32_t>(
        reinterpret_cast<std::uint32_t *>(DataPtr), MaxEvents);
    SizePtr = reinterpret_cast<flatbuffers::uoffset_t *>(
                  const_cast<std::uint8_t *>(DataPtr)) -
              1;
    *SizePtr = 0;
  }
  void clear() { *SizePtr = 0; }
  void push_back(std::uint32_t Value) {
    Data[*SizePtr] = Value;
    (*SizePtr)++;
  }
  flatbuffers::uoffset_t Offset{0};
  std::uint8_t *DataPtr{nullptr};
  nonstd::span<std::uint32_t> Data;
  flatbuffers::uoffset_t *SizePtr{nullptr};
};

using namespace std::chrono_literals;

void EventSerializer::serialiseThreadFunc() {
  flatbuffers::FlatBufferBuilder Builder(
      sizeof(std::uint32_t) * EventBufferSize * 8 + 2048);
  // We do not want the buffer to be too small or the vector offset addresses
  // will become invalid when creating them.
  auto SourceNameOffset = Builder.CreateString(Name);
  auto TimeOffset = FBVector(Builder, EventBufferSize);
  auto EventId = FBVector(Builder, EventBufferSize);
  auto Amplitude = FBVector(Builder, EventBufferSize);
  auto PeakArea = FBVector(Builder, EventBufferSize);
  auto Background = FBVector(Builder, EventBufferSize);
  auto ThresholdTime = FBVector(Builder, EventBufferSize);
  auto PeakTime = FBVector(Builder, EventBufferSize);

  auto AdcPulseDebugOffset = CreateAdcPulseDebug(
      Builder, Amplitude.Offset, PeakArea.Offset, Background.Offset,
      ThresholdTime.Offset, PeakTime.Offset);
  auto FBMessage = CreateEventMessage(
      Builder, SourceNameOffset, 1, 1, TimeOffset.Offset, EventId.Offset,
      FacilityData::AdcPulseDebug, AdcPulseDebugOffset.Union());
  FinishEventMessageBuffer(Builder, FBMessage);
  auto EventMessage = GetMutableEventMessage(Builder.GetBufferPointer());

  std::unique_ptr<EventData> NewEvent;
  std::uint64_t MessageId{0};
  std::chrono::system_clock::time_point FirstEventTime;
  EventBuffer Events(EventBufferSize);

  auto ProduceFB = [&](auto const &SendEvents, auto const ReferenceTime) {
    if (SendEvents.empty()) {
      return;
    }
    EventMessage->mutate_message_id(MessageId++);
    EventMessage->mutate_pulse_time(
        RefTimeOffset.calcTimestampNS(ReferenceTime));

    for (auto const &CEvent : SendEvents) {
      TimeOffset.push_back(
          static_cast<std::uint32_t>(CEvent.Timestamp - ReferenceTime));
      ThresholdTime.push_back(
          RefTimeOffset.calcTimestampNS(CEvent.ThresholdTime));
      PeakTime.push_back(RefTimeOffset.calcTimestampNS(CEvent.PeakTime));
      EventId.push_back(CEvent.EventId);
      Amplitude.push_back(CEvent.Amplitude);
      PeakArea.push_back(CEvent.PeakArea);
      Background.push_back(CEvent.Background);
    }

    Producer->produce({Builder.GetBufferPointer(), Builder.GetSize()},
                      RefTimeOffset.calcTimestampNS(ReferenceTime) / 1000000);
    TimeOffset.clear();
    EventId.clear();
    Amplitude.clear();
    PeakArea.clear();
    Background.clear();
    ThresholdTime.clear();
    PeakTime.clear();
  };

  std::uint64_t LastAddedRefTimestamp{0};
  auto getReferenceTimestamps = [&LastAddedRefTimestamp, &Events, this]() {
    auto NumberOfIterations{0};
    auto const MaxIterations{40};
    std::uint64_t TempRefTime{0};
    while (this->ReferenceTimeQueue.try_dequeue(TempRefTime)) {
      if (TempRefTime != LastAddedRefTimestamp) {
        Events.addReferenceTimestamp(TempRefTime);
        LastAddedRefTimestamp = TempRefTime;
      }
      ++NumberOfIterations;
      if (NumberOfIterations > MaxIterations) {
        break;
      }
    }
  };
  getReferenceTimestamps();

  do {
    while (EventQueue.try_dequeue(NewEvent)) {
      if (Events.size() == 0) {
        FirstEventTime = getCurrentTime();
      }
      Events.addEvent(NewEvent);
      getReferenceTimestamps();
      auto EventList = Events.getFrameEvents();
      if (not EventList.Events.empty()) {
        ProduceFB(EventList.Events, EventList.RefTime);
        Events.cullEvents(EventList.Events.size());
        if (Events.size() > 0) {
          FirstEventTime = getCurrentTime();
        }
      }
    }
    std::this_thread::sleep_for(
        1ms); // Maybe remove this wait in case we get a really high event rate
    getReferenceTimestamps();
    if (Events.size() > 0 and getCurrentTime() > FirstEventTime + Timeout) {
      auto EventList = Events.getAllEvents();
      ProduceFB(EventList.Events, EventList.RefTime);
      Events.clearAllEvents();
    }
  } while (RunThread);

  if (Events.size() > 0) {
    // Reference time is calculated by Events.getEvents() so we must call that
    // function first.
    auto EventList = Events.getFrameEvents();
    if (EventList.Events.empty()) {
      EventList = Events.getAllEvents();
    }
    ProduceFB(EventList.Events, EventList.RefTime);
  }
}
