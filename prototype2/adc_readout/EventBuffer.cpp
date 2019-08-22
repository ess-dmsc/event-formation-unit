// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include "EventBuffer.h"


EventBuffer::EventBuffer(size_t BufferSize) : Events(BufferSize), MaxSize(BufferSize) {
  Events.reserve(MaxSize);
}

bool EventBuffer::addEvent(std::unique_ptr<EventData> const &Event) {
  if (Size >= MaxSize) {
    return false;
  }
  Events[Size] = *Event;
  ++Size;
  return true;
}

bool EventBuffer::shouldCullEvents() {
  if (Size == 0) {
    return false;
  }
  auto UsedRefTime = RefTime;
  if (RefTime == 0 or RefTime + MaxOffsetTime < Events[0].Timestamp) {
    UsedRefTime = Events[0].Timestamp;
  }
  if (Events[Size - 1].Timestamp > UsedRefTime +  MaxOffsetTime) {
    return true;
  }
  return Size == MaxSize;
}

void EventBuffer::clearEvents() {
  auto ShiftElement = 0;
  auto UsedRefTime = RefTime;
  if (RefTime == 0 or RefTime + MaxOffsetTime < Events[0].Timestamp) {
    UsedRefTime = Events[0].Timestamp;
  }
  for (auto Iter = Events.begin(); Iter != Events.begin() + Size; ++Iter) {
    if (Iter->Timestamp > UsedRefTime + MaxOffsetTime) {
      break;
    }
    ++ShiftElement;
  }
  std::move(Events.begin() + ShiftElement, Events.begin() + Size, Events.begin());
  Size = Size - ShiftElement;
}

nonstd::span<EventData const> EventBuffer::getEvents() {
  size_t ReturnSize{0};
  if (Size == 0) {
    return {};
  }
  auto UsedRefTime = RefTime;
  if (RefTime == 0 or RefTime + MaxOffsetTime < Events[0].Timestamp) {
    UsedRefTime = Events[0].Timestamp;
  }
  for (auto Iter = Events.begin(); Iter != Events.begin() + Size; ++Iter) {
    if (Iter->Timestamp > UsedRefTime + MaxOffsetTime) {
      break;
    }
    ++ReturnSize;
  }
  return nonstd::span<EventData const>(Events.data(), ReturnSize);
}

void EventBuffer::setReferenceTimes(std::uint64_t ReferenceTime, std::uint64_t Timespan) {
  RefTime = ReferenceTime;
  MaxOffsetTime = Timespan;
}

nonstd::span<EventData const> EventBuffer::getAllEvents() {
  return nonstd::span<EventData const>(Events.data(), Size);
}

void EventBuffer::clearAllEvents() {
  Size = 0;
}
