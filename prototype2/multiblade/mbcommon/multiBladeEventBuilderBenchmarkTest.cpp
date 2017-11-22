/** Copyright (C) 2017 European Spallation Source ERIC */

#include <benchmark/benchmark.h>
#include <multiblade/mbcommon/multiBladeEventBuilder.h>
#include <string>
#include <unistd.h>

/** @file
 *
 *  @brief Reading null data
 * File:
 */

multiBladeEventBuilder mbevb;
unsigned char databuffer[100000];

static void Setup(__attribute__((unused)) benchmark::State& state) {
  memset(databuffer, 0, sizeof(databuffer));
}
BENCHMARK(Setup);

static void AddDataPoint(benchmark::State& state) {
  Setup(state);
  uint32_t items = 0;

  for (auto _ : state) {
    for (int i = 0; i < state.range(0); i++) {
       mbevb.addDataPoint(i, 555, i);
     }
     mbevb.addDataPoint(1, 777, 2000); // outside time window
     items += state.range(0); // number of data points
  }
  state.SetComplexityN(state.range(0));
  state.SetBytesProcessed(state.iterations() * state.range(0));
  state.SetItemsProcessed(items);
};
BENCHMARK(AddDataPoint)->RangeMultiplier(2)->Range(2, 64)->Complexity();

BENCHMARK_MAIN();
