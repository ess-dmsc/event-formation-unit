// Copyright (C) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file DetectorType.h
///
/// Detector enums used in the EFU
//===----------------------------------------------------------------------===//

#pragma once

#include <magic_enum/magic_enum.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

class DetectorType {
public:

  // Enum definition
  enum Types {
    RESERVED = 0x00,
    CBM      = 0x10,
    LOKI     = 0x30,
    TBL3HE   = 0x32,
    BIFROST  = 0x34,
    MIRACLES = 0x38,
    CSPEC    = 0x3C,
    TREX     = 0x40,
    NMX      = 0x44,
    FREIA    = 0x48,
    TBLMB    = 0x49,
    ESTIA    = 0x4C,
    BEER     = 0x50,
    DREAM    = 0x60,
    MAGIC    = 0x64,
    HEIMDAL  = 0x68
  };

  // Max and min enum values
  static constexpr int MIN = Types::RESERVED;
  static constexpr int MAX = Types::HEIMDAL;

  // Construct from string
  DetectorType(const std::string &typeName) {
    // Convert to upper case, so both "value" and "VALUE" will work
    std::string upper = typeName;
    std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c){ return std::toupper(c); });

    const auto t = magic_enum::enum_cast<Types>(upper);
    if (t.has_value()) {
      mDetectorType = t.value();
    } else {
      throw std::out_of_range("Invalid DetectorType string: " + typeName);
    }
  }

  // Construct from integer
  DetectorType(const int type=RESERVED) {
    if (type >= MIN && type <= MAX) {
      mDetectorType = static_cast<Types>(type);
    }

    else {
      throw std::out_of_range("Invalid DetectorType integer: " +
                                  std::to_string(type));
    }
  }

  std::string toString() const {
    const std::string name(magic_enum::enum_name(mDetectorType));

    return name;
  }

  std::string toUpperCase() const {
    return toString();
  }

  std::string toLowerCase() const {
    // Convert to lower case
    std::string lower = toString();
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return std::tolower(c); });

    return lower;
  }

  operator uint8_t() const { return static_cast<uint8_t>(mDetectorType); }

  bool operator==(const DetectorType &other) const {
    return mDetectorType == other.mDetectorType;
  }

  bool operator!=(const DetectorType &other) const {
    return mDetectorType != other.mDetectorType;
  }

  bool operator==(const Types &other) const {
    return mDetectorType == other;
  }

  bool operator!=(const Types &other) const {
    return mDetectorType != other;
  }

private:
  Types mDetectorType;
};
