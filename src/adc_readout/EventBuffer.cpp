// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include "EventBuffer.h"
#include <algorithm>

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
  std::move(Events.begin() + NrOfEvents, Events.begin() + Size, Events.begin());
  Size = Size - NrOfEvents;
}

std::pair<EventList, std::uint64_t> EventBuffer::getFrameEvents() {
  if (Size == 0) {
    return {};
  }
  // If we are going outside of our MaxOffsetTime, do not use the current
  // reference time.
  auto FirstEventTime = Events[0].Timestamp;
  if (RefTime + MaxOffsetTime < FirstEventTime) {
    RefTime = FirstEventTime;
  }

  for (size_t i = ReferenceTimestamps.size(); i > 0; --i) {
    if (ReferenceTimestamps[i - 1] <= FirstEventTime and
        ReferenceTimestamps[i - 1] + MaxOffsetTime > FirstEventTime) {
      RefTime = ReferenceTimestamps[i - 1];
      ReferenceTimestamps.erase(ReferenceTimestamps.begin(),
                                ReferenceTimestamps.begin() + i);
      break;
    }
  }

  ReferenceTimestamps.erase(std::remove_if(ReferenceTimestamps.begin(),
                                           ReferenceTimestamps.end(),
                                           [FirstEventTime](auto Time) {
                                             return Time < FirstEventTime;
                                           }),
                            ReferenceTimestamps.end());

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
  return {}; // If buffer is not full or we are not out of our time range, don't
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
  if (NewReferenceTime != RefTime) {
    if (ReferenceTimestamps.empty()) {
      ReferenceTimestamps.push_back(NewReferenceTime);
    } else if (std::find(ReferenceTimestamps.begin(), ReferenceTimestamps.end(),
                         NewReferenceTime) == ReferenceTimestamps.end()) {
      ReferenceTimestamps.push_back(NewReferenceTime);
      std::sort(ReferenceTimestamps.begin(), ReferenceTimestamps.end());
    }
  }
}
