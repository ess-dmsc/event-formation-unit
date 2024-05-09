// Copyright (C) 2024 European Spallation Source, see LICENSE file
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
#include <chrono>

// #ifndef NDEBUG
#include <future>
#include <functional>
// #endif

using namespace std::chrono;

namespace efutils {

template <typename Func, typename... Args>
int inline measureRuntime(Func &&func, Args &&...args) {

// #ifndef NDEBUG
  auto startTime = std::chrono::high_resolution_clock::now();
// #endif
  std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
// #ifndef NDEBUG
  auto endTime = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime)
          .count();

  return duration;
// #else
//   return 0;
// #endif
}

inline nanoseconds hzToNanoseconds(const float &frequencyHz) {
  uint64_t nsPeriod = static_cast<uint64_t>(1e9 / frequencyHz);
  return nanoseconds(nsPeriod);
}

inline microseconds nsToMicrosecons(const uint64_t &nanoseconds) {
  return microseconds(nanoseconds / 1000);
}

inline milliseconds nsToMilliseconds(uint64_t nanoseconds) {
  return milliseconds(nanoseconds / 1000000);
}

} // namespace efutils