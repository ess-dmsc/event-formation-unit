/** Copyright (C) 2018 European Spallation Source ERIC */

#include <benchmark/benchmark.h>
#include <logical_geometry/ESSGeometry.h>
#include <string>
#include <unistd.h>


static void GetPixelID(benchmark::State &state) {
  uint32_t items = 0;

  ESSGeometry essgeom(100, 100, 100, 100);

  for (auto _ : state) {
    for (size_t i = 10; i < 20; i++) {
      benchmark::DoNotOptimize(essgeom.pixelMP3D(i,i,i,i));
      items += 1;
    }
  }
  state.SetItemsProcessed(items);
}
BENCHMARK(GetPixelID);

BENCHMARK_MAIN();
