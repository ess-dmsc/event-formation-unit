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
  OffsetTime(Offset Time, std::chrono::system_clock::time_point OffsetPoint =
                              std::chrono::system_clock::now());
  explicit OffsetTime(std::chrono::system_clock::time_point OffsetPoint);
  std::uint64_t calcTimestamp(std::uint64_t InputTime);

private:
  Offset OffsetSetting{Offset::NONE};
  std::uint64_t UsedOffset{0};
  std::chrono::system_clock::time_point StartTime;
};
