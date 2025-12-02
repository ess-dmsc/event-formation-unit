// Copyright (C) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file TimeSource.h
///
/// Time source enums used by the EFU
//===----------------------------------------------------------------------===//

#pragma once

#include <magic_enum/magic_enum.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <string>

class TimeSource {
public:

  // Enum definition
  enum Types {
    TIME_SOURCE       = 1 << 0,
    SYNC_SOURCE       = 1 << 1,
    LOCAL_SYNC_SOURCE = 1 << 2,
    TIMING_STATUS     = 1 << 3,

    EVEN_FIBRE_STATUS = 1 << 4,
    EVEN_FIBRE_SYNC   = 1 << 5,

    ODD_FIBRE_STATUS  = 1 << 6,
    ODD_FIBRE_SYNC    = 1 << 7,
  };

  // Max and min enum values
  static constexpr int MIN = Types::TIME_SOURCE;
  static constexpr int MAX = Types::ODD_FIBRE_SYNC;

  // Construct from string
  TimeSource(const std::string &typeName) {

    // Convert to upper case, so both "value" and "VALUE" will work
    std::string upper = typeName;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    const auto t = magic_enum::enum_cast<Types>(upper);
    if (t.has_value()) {
      mTimeSource = t.value();
    } else {
      throw std::out_of_range("Invalid TimeSource string: " + typeName);
    }
  }

  // Construct from integer
  TimeSource(const int type=TIME_SOURCE) {
    if (type >= MIN && type <= MAX) {
      mTimeSource = static_cast<Types>(type);
    }

    else {
      throw std::invalid_argument("Invalid TimeSource integer: " +
                                  std::to_string(type));
    }
  }

  std::string toString() const {
    const std::string name(magic_enum::enum_name(mTimeSource));

    return name;
  }

  std::string toUpperCase() const {
    return toString();
  }

  std::string toLowerCase() const {
    // Convert to lower case
    std::string lower = toString();
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    return lower;
  }

  operator int() const { return static_cast<int>(mTimeSource); }

  bool operator==(const TimeSource &other) const {
    return mTimeSource == other.mTimeSource;
  }

  bool operator!=(const TimeSource &other) const {
    return mTimeSource != other.mTimeSource;
  }

  bool operator==(const Types &other) const {
    return mTimeSource == other;
  }

  bool operator!=(const Types &other) const {
    return mTimeSource != other;
  }

private:
  Types mTimeSource;
};
