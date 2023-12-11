// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Utility file with functions useful for various unit conversion
//===----------------------------------------------------------------------===//

#pragma once

#include <chrono>
#include <cstdint>
#include <ratio>

using namespace std::chrono;

namespace efutils {

inline nanoseconds hzToNanoseconds(const int &frequencyHz) {
  uint64_t nsPeriod = static_cast<uint64_t>(1e9 / frequencyHz);
  return nanoseconds(nsPeriod);
}

inline microseconds nsToMicrosecons(const uint64_t &nanoseconds) {
  return microseconds(nanoseconds / 1000);
}

inline milliseconds nsToMilliseconds(uint64_t nanoseconds) {
    return milliseconds(nanoseconds / 1000000);
}

} // namespace Utils