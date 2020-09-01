// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include "EventData.h"
#include <common/span.hpp>
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
  explicit EventBuffer(size_t BufferSize);
  /// \brief Add event to buffer.
  ///
  /// \param Event The event that should be added
  /// \return Returns true on success, false if buffer is already full.
  bool addEvent(std::unique_ptr<EventData> const &Event);
  void addReferenceTimestamp(std::uint64_t NewReferenceTime);
  void setTimespan(std::uint64_t NewTimespan) { MaxOffsetTime = NewTimespan; }

  struct EventsTimeData {
    EventList Events;
    std::uint64_t RefTime;
  };

  EventsTimeData getFrameEvents();

  /// \brief Get all events from the buffer.
  EventsTimeData getAllEvents() const;
  /// \brief Remove events from the buffer.
  /// \param[in] NrOfEvents The nr of events from the start of the buffer to
  /// remove.
  void cullEvents(size_t NrOfEvents);
  /// \brief Remove all events.
  void clearAllEvents();
  /// \brief Get the number of events in the buffer.
  size_t size() const { return Size; }

private:
  std::vector<EventData> Events;
  std::deque<std::uint64_t> ReferenceTimestamps;
  const size_t MaxSize;
  size_t Size{0};
  std::uint64_t RefTime{0};
  std::uint64_t MaxOffsetTime{std::numeric_limits<std::uint32_t>::max()};
};
