#include <benchmark/benchmark.h>
#include <jalousie/geometry/DreamGeometry.h>

#include <common/BenchmarkUtil.h>

const uint32_t Count = DreamGeometry::TotalPixels / 10;

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
  state.SetBytesProcessed(Total * (sizeof(EndCapParams) + sizeof(SlicePixel) + sizeof(StripPlanePixel)));
}
BENCHMARK(DreamDecode);

__attribute__((noinline)) 
void EndCapParamsFromPixelId_Bulk(uint32_t TheCount) {
  for (uint32_t PixelId = 1; PixelId < TheCount; PixelId++) {
    EndCapParams EndCap = EndCapParamsFromPixelId(PixelId);
    ::benchmark::ClobberMemory();
    ::benchmark::DoNotOptimize(EndCap);
  }
}

static void DreamDecode_Bulk(benchmark::State &state) {
  uint32_t Total = 0;
  BenchmarkLoop(state, [&] {
    EndCapParamsFromPixelId_Bulk(Count);
    Total += Count;
  });
  state.SetItemsProcessed(Total);
  state.SetBytesProcessed(Total * (sizeof(EndCapParams) + sizeof(SlicePixel) + sizeof(StripPlanePixel)));
}
BENCHMARK(DreamDecode_Bulk);

BENCHMARK_MAIN();
