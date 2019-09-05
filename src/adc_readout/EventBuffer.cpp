// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include "EventBuffer.h"

EventBuffer::EventBuffer(size_t BufferSize)
    : Events(BufferSize), MaxSize(BufferSize) {
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

void EventBuffer::cullEvents(size_t NrOfEvents) {
  std::move(Events.begin() + NrOfEvents, Events.begin() + Size,
            Events.begin());
  Size = Size - NrOfEvents;
}

std::pair<EventList, std::uint64_t> EventBuffer::getEvents() {
  if (Size == 0) {
    return {};
  }
  auto FirstEventTime = Events[0].Timestamp;
  if (RefTime == 0 or RefTime + MaxOffsetTime < FirstEventTime) {
    RefTime = FirstEventTime;
  }
  for (size_t i = ReferenceTimestamps.size(); i > 0; --i) {
    if (ReferenceTimestamps[i - 1] < FirstEventTime and
        ReferenceTimestamps[i - 1] + MaxOffsetTime > FirstEventTime) {
      RefTime = ReferenceTimestamps[i - 1];
      ReferenceTimestamps.erase(ReferenceTimestamps.begin(),
                                ReferenceTimestamps.begin() + i);
      break;
    }
  }
  
  while (not ReferenceTimestamps.empty() and ReferenceTimestamps.front() < FirstEventTime) {
    ReferenceTimestamps.pop_front();
  }
  
  if (MaxSize == Size) {
    return {nonstd::span<EventData const>(Events.data(), Size), RefTime};
  }
  
  size_t ReturnSize{0};
  for (auto Iter = Events.begin(); Iter != Events.begin() + Size; ++Iter) {
    if (ReferenceTimestamps.empty()) {
      if (Iter->Timestamp > (RefTime + MaxOffsetTime)) {
        return {nonstd::span<EventData const>(Events.data(), ReturnSize),
          RefTime};
      }
    } else {
      if (Iter->Timestamp >= ReferenceTimestamps.front()) {
        return {nonstd::span<EventData const>(Events.data(), ReturnSize),
          RefTime};
      }
    }
    ++ReturnSize;
  }
  return {}; // If buffer is not full or we are not out of our time range, dont
             // return any events
}

std::pair<EventList, std::uint64_t> EventBuffer::getAllEvents() const {
  if (Size == 0) {
    return {};
  }
  return {nonstd::span<EventData const>(Events.data(), Size), RefTime};
}

void EventBuffer::clearAllEvents() { Size = 0; }

void EventBuffer::addReferenceTimestamp(std::uint64_t NewReferenceTime) {
  if (RefTime == 0) {
    RefTime = NewReferenceTime;
  } if (ReferenceTimestamps.empty())   {
    ReferenceTimestamps.push_back(NewReferenceTime);
  } else if (ReferenceTimestamps.back() != NewReferenceTime) {
    ReferenceTimestamps.push_back(NewReferenceTime);
  }
}
