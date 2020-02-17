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

// main benchmark loop. Replaces the 'for (auto _ : state){...}'.
// usage: 'BenchmarkLoop(state, [&]{ MyFancyCode });'
template <typename Lambda>
void BenchmarkLoop(benchmark::State &state, Lambda loopBody) {
  BenchmarkOuterStartTiming();
  for (auto _ : state) {
    loopBody();
  }
  BenchmarkOuterStopTiming();
}

// used for running setup/data generation code during a test. Adds
// overhead of 1000 ns in tests.
template <typename Lambda>
void BenchmarkLoopPaused(benchmark::State &state, Lambda runWithoutTiming) {
  BenchmarkLoopPauseTiming(state);
  runWithoutTiming();
  BenchmarkLoopResumeTiming(state);
}