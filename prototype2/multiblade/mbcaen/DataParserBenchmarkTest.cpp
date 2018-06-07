/** Copyright (C) 2017 European Spallation Source ERIC */

#include <benchmark/benchmark.h>
#include <multiblade/mbcaen/MBData.h>
#include <cstring>
#include <unistd.h>

/** @file
 *
 *  @brief Reading null data
 * File:
 */

MBData mbdata;
unsigned char databuffer[100000];

static void Setup(__attribute__((unused)) benchmark::State &state) {
  memset(databuffer, 0, sizeof(databuffer));
}
BENCHMARK(Setup);

static void ReceiveData(benchmark::State &state) {
  Setup(state);
  uint32_t items = 0;

  for (auto _ : state) {
    auto ret = mbdata.receive((char *)databuffer, state.range(0));
    items += ret;
  }
  state.SetComplexityN(state.range(0));
  state.SetBytesProcessed(state.iterations() * state.range(0));
  state.SetItemsProcessed(items);
}
BENCHMARK(ReceiveData)->RangeMultiplier(2)->Range(8, 80 << 10)->Complexity();

BENCHMARK_MAIN();
