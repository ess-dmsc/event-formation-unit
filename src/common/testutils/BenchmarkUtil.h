// Copyright (C) 2020-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This utility supports various profiler/timer instrumentation in
///        Google Benchmark
///
/// It does this by adding the BenchmarkLoop() and
/// BenchmarkLoopPaused() functions.
/// Both functions allow the user to execute their code in a lambda,
/// while turning instrumentation on/off at the correct times.
//===----------------------------------------------------------------------===//
#pragma once

#include <benchmark/benchmark.h>

#ifdef BUILD_SUPPORT_VALGRIND_SDK
#include <valgrind/callgrind.h>
#endif

// start timing in outer benchmark scope, once initial setup is once
inline void BenchmarkOuterStartTiming() {
#ifdef BUILD_SUPPORT_VALGRIND_SDK
  CALLGRIND_TOGGLE_COLLECT; // turn on
#endif
}

inline void BenchmarkOuterStopTiming() {
#ifdef BUILD_SUPPORT_VALGRIND_SDK
  CALLGRIND_TOGGLE_COLLECT; // turn off
#endif
}

// stop timing in inner benchmark loop, so test data can be created. Adds
// overhead of 1000 ns in tests.
inline void BenchmarkLoopPauseTiming(benchmark::State &state) {
  state.PauseTiming();
#ifdef BUILD_SUPPORT_VALGRIND_SDK
  CALLGRIND_TOGGLE_COLLECT; // turn off
#endif
}

inline void BenchmarkLoopResumeTiming(benchmark::State &state) {
#ifdef BUILD_SUPPORT_VALGRIND_SDK
  CALLGRIND_TOGGLE_COLLECT; // turn on
#endif
  state.ResumeTiming();
}

/// \brief Main benchmark loop. Replaces the 'for (auto _ : state){...}'.
///        Usage: 'BenchmarkLoop(state, [&]{ MyFancyCode });'
template <typename Lambda>
void BenchmarkLoop(benchmark::State &state, Lambda loopBody) {
  BenchmarkOuterStartTiming();
  for (auto _ : state) {
    loopBody();
  }
  BenchmarkOuterStopTiming();
}

/// \brief Used for running setup/data generation code during a test. Adds
///        overhead of 1000 ns in tests.
/// Usage:
///        BenchmarkLoop(state, [&] {
///           BenchmarkLoopPaused(state, [&] {
///             ... Initialize and build data arrays, etc. here ...
///           });
///        });
template <typename Lambda>
void BenchmarkLoopPaused(benchmark::State &state, Lambda runWithoutTiming) {
  BenchmarkLoopPauseTiming(state);
  runWithoutTiming();
  BenchmarkLoopResumeTiming(state);
}
