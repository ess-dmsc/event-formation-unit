// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include "EventData.h"
#include "common/span.hpp"
#include <deque>
#include <limits>
#include <memory>
#include <stddef.h>
#include <vector>

using EventList = nonstd::span<EventData const>;

/// \brief Simple event buffer that keeps track of events based on their
/// timestamps
class EventBuffer {
public:
  /// \brief Initializes the event buffer.
  ///
  /// \param BufferSize The maximum number events in the buffer.
  EventBuffer(size_t BufferSize);
  /// \brief Add event to buffer.
  ///
  /// \param Event The event that should be added
  /// \return Returns true on success, false if buffer is already full.
  bool addEvent(std::unique_ptr<EventData> const &Event);
  void addReferenceTimestamp(std::uint64_t NewReferenceTime);
  void setTimespan(std::uint64_t NewTimespan) { MaxOffsetTime = NewTimespan; }
  std::pair<EventList, std::uint64_t> getEvents();
  /// \brief Clear events from the buffer that fall within the range of
  /// ReferenceTime to ReferenceTime + Timespan
  std::pair<EventList, std::uint64_t> getAllEvents() const;
  void cullEvents(size_t NrOfEvents);
  void clearAllEvents();
  size_t size() const { return Size; }

private:
  std::vector<EventData> Events;
  std::deque<std::uint64_t> ReferenceTimestamps;
  const size_t MaxSize;
  size_t Size{0};
  std::uint64_t RefTime{0};
  std::uint64_t MaxOffsetTime{std::numeric_limits<std::uint32_t>::max()};
};
