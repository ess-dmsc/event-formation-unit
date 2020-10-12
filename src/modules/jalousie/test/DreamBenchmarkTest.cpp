#include <benchmark/benchmark.h>
#include <jalousie/geometry/DreamGeometry.h>

#include <common/BenchmarkUtil.h>

const uint32_t Count = TotalPixels / 10;

static void DreamDecode(benchmark::State &state) {
  uint32_t Total = 0;
  BenchmarkLoop(state, [&] {
    for (uint32_t PixelId = 1; PixelId < Count; PixelId++) {
      EndCapParams EndCap = EndCapParamsFromPixelId(PixelId);
      ::benchmark::ClobberMemory();
      ::benchmark::DoNotOptimize(EndCap);
    }
    Total += Count;
  });
  state.SetItemsProcessed(Total);
}
BENCHMARK(DreamDecode);

__attribute__((noinline)) void
EndCapParamsFromPixelId_Bulk() {
  for (uint32_t PixelId = 1; PixelId < Count; PixelId++) {
    EndCapParams EndCap = EndCapParamsFromPixelId(PixelId);
    ::benchmark::ClobberMemory();
    ::benchmark::DoNotOptimize(EndCap);
  }
}

static void DreamDecode_Bulk(benchmark::State &state) {
  uint32_t Total = 0;
  BenchmarkLoop(state, [&] {
    EndCapParamsFromPixelId_Bulk();
    Total += Count;
  });
  state.SetItemsProcessed(Total);
}
BENCHMARK(DreamDecode_Bulk);

BENCHMARK_MAIN();
