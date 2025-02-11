// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Time related debug methods and classes
///
//===----------------------------------------------------------------------===//

#pragma once

// Only use these if we are in debug mode
// because they used in a time measurement code
// for debugging purposes
// #ifndef NDEBUG
#include <functional>
#include <future>
// #endif

namespace time_debug {

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

} // namespace time_debug