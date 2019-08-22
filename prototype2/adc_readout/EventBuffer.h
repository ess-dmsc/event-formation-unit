// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include "EventData.h"
#include <stddef.h>
#include "common/span.hpp"
#include <memory>
#include <vector>
#include <limits>

/// \brief Simple event buffer that keeps track of events based on their timestamps
class EventBuffer {
public:
  /// \brief Initializes the event buffer.
  ///
  /// \param BufferSize The maximum number events in the buffer.
  EventBuffer(size_t BufferSize);
  /// \brief Used for setting the reference timestamps for determining if the buffer should be culled/emptied.
  ///
  /// \param ReferenceTime Reference timestamp. If set to 0, uses the timestamp of the first event.
  /// \param Timespan The accpetable timespan (offset from ref. time or first time stamp).
  void setReferenceTimes(std::uint64_t ReferenceTime, std::uint64_t Timespan);
  /// \brief Add event to buffer.
  ///
  /// \param Event The event that should be added
  /// \return Returns true on success, false if buffer is already full.
  bool addEvent(std::unique_ptr<EventData> const &Event);
  bool shouldCullEvents();
  nonstd::span<EventData const> getEvents();
  /// \brief Clear events from the buffer that fall within the range of ReferenceTime to ReferenceTime + Timespan
  nonstd::span<EventData const> getAllEvents();
  void clearEvents();
  void clearAllEvents();
private:
  std::vector<EventData> Events;
  const size_t MaxSize;
  size_t Size{0};
  std::uint64_t RefTime{0};
  std::uint64_t MaxOffsetTime{std::numeric_limits<std::uint32_t>::max()};
};
