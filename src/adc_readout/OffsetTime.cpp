// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate offset for reference timestamps.
///
//===----------------------------------------------------------------------===//

#include "OffsetTime.h"

OffsetTime::OffsetTime(OffsetTime::Offset Time,
                       std::chrono::system_clock::time_point ReferenceTimePoint,
                       std::uint64_t StartTimestampNS) {
  if (Time == Offset::NONE) {
    return; // Do nothing
  } else if (Time == Offset::NOW) {
    ReferenceTimePoint = std::chrono::system_clock::now();
  }
  auto RefTimePointNS = std::chrono::duration_cast<std::chrono::nanoseconds>(
                            ReferenceTimePoint.time_since_epoch())
                            .count();
  UsedOffset = RefTimePointNS - StartTimestampNS;
}
