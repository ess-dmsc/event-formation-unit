/** Copyright (C) 2018 European Spallation Source ERIC */

#include <benchmark/benchmark.h>
#include <stdio.h>
#include <unistd.h>
#include <libs/include/BitMath.h>
#include <test/TestBase.h>


static const int iterations{10000};

/// Implementation used at CERN for Atlas
/// Appears to be tree times as slow than the once
/// we use from BitMath (on Mac)
inline unsigned int grayToBinary_CERN_Atlas(unsigned int num)
{
    uint mask;
    for( mask = num >> 1; mask != 0; mask = mask >> 1)
    {
        num = num ^ mask;
    }
    return num;
}

static void Benchmark_Atlas(benchmark::State &state)
{
	int items = 0;
  for (auto _ : state) {
		for (int i = 0; i < iterations; i++)
		  benchmark::DoNotOptimize(grayToBinary_CERN_Atlas(i));
    items += iterations;
  }
  state.SetItemsProcessed(items);
}
BENCHMARK(Benchmark_Atlas);

static void Benchmark_BitMath(benchmark::State &state)
{
	int items = 0;
  for (auto _ : state) {
		for (int i = 0; i < iterations; i++)
			benchmark::DoNotOptimize(BitMath::gray2bin32(i));
    items += iterations;
  }
  state.SetItemsProcessed(items);
}
BENCHMARK(Benchmark_BitMath);

BENCHMARK_MAIN();
