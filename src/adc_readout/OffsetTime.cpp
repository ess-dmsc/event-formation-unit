// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate offset for reference timestamps.
///
//===----------------------------------------------------------------------===//

#include "OffsetTime.h"

OffsetTime::OffsetTime(OffsetTime::Offset Time, std::chrono::system_clock::time_point OffsetPoint) : OffsetSetting(Time), StartTime(OffsetPoint) {

}

OffsetTime::OffsetTime(std::chrono::system_clock::time_point OffsetPoint) : OffsetSetting(OffsetTime::TIME_POINT), StartTime(OffsetPoint) {

}

std::uint64_t OffsetTime::calcTimestamp(std::uint64_t InputTime) {
  if ((OffsetSetting == NOW or OffsetSetting == TIME_POINT) and UsedOffset == 0) {
    auto TimePointNS = std::chrono::duration_cast<std::chrono::nanoseconds>(StartTime.time_since_epoch()).count();
    UsedOffset = TimePointNS - InputTime;
  }
  return InputTime + UsedOffset;
}