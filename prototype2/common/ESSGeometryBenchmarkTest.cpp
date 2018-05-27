/** Copyright (C) 2018 European Spallation Source ERIC */

#include <benchmark/benchmark.h>
#include <common/ESSGeometry.h>
#include <gdgem/nmx/Geometry.h>
#include <string>
#include <unistd.h>


static void GetPixelID(benchmark::State &state) {
  uint32_t items = 0;

  ESSGeometry essgeom(100, 100, 100, 100);

  for (auto _ : state) {
    for (size_t i = 10; i < 20; i++) {
      benchmark::DoNotOptimize(essgeom.getPixelMP3D(i,i,i,i));
      items += 1;
    }
  }
  state.SetItemsProcessed(items);
};
BENCHMARK(GetPixelID);


static void GetPixelIDNMX(benchmark::State &state) {
  uint32_t items = 0;
  std::vector<uint16_t> coords{0, 0, 0, 0};

  Geometry geom;
  geom.add_dimension(100);
  geom.add_dimension(100);
  geom.add_dimension(100);
  geom.add_dimension(100);

  for (auto _ : state) {
    for (size_t i = 10; i < 20; i++) {
      coords[0] = i;
      coords[1] = i;
      coords[2] = i;
      coords[2] = i;
      benchmark::DoNotOptimize(geom.to_pixid(coords));
      items += 1;
    }
  }
  state.SetItemsProcessed(items);
};
BENCHMARK(GetPixelIDNMX);


BENCHMARK_MAIN();
