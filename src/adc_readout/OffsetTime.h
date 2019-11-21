// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate offset for reference timestamps.
///
//===----------------------------------------------------------------------===//

#pragma once

#include <chrono>

class OffsetTime {
public:
  enum class Offset { NONE, NOW, TIME_POINT };
  static const Offset NOW = Offset::NOW;
  static const Offset NONE = Offset::NONE;
  static const Offset TIME_POINT = Offset::TIME_POINT;
  OffsetTime(Offset Time, std::chrono::system_clock::time_point ReferenceTimePoint =
                              std::chrono::system_clock::now(), std::uint64_t StartTimestampNS = 0);
  std::uint64_t calcTimestampNS(std::uint64_t InputTimeNS) {
    return InputTimeNS + UsedOffset;
  }
private:
  std::int64_t UsedOffset{0};
};
