// Copyright (C) 2024 - 2025 European Spallation Source, see LICENSE file
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

// #ifndef NDEBUG
#include <functional>
#include <future>
// #endif

using namespace std::chrono;

namespace efutils {

/// \brief Measure the runtime of a function
/// \param func The function to measure
/// \param args The arguments to the function
/// \return The runtime of the function in microseconds
/// \note This function is only active in debug mode
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

inline microseconds nsToMicroseconds(const uint64_t &nanoseconds) {
  return std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::nanoseconds(nanoseconds));
}

inline milliseconds nsToMilliseconds(const uint64_t &nanoseconds) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::nanoseconds(nanoseconds));
}

inline nanoseconds sToNanoseconds(const uint64_t &seconds) {
  return std::chrono ::duration_cast<std::chrono::nanoseconds>(
      std::chrono::seconds(seconds));
}

inline milliseconds sToMilliseconds(const uint64_t &seconds) {
  return std::chrono ::duration_cast<std::chrono::milliseconds>(
      std::chrono::seconds(seconds));
}

} // namespace efutils