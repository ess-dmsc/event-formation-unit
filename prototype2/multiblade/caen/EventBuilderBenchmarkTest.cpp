/** Copyright (C) 2019 European Spallation Source ERIC */

#include <multiblade/clustering/EventBuilder.h>
#include <benchmark/benchmark.h>
#include <string>
#include <unistd.h>

Multiblade::EventBuilder builder;

// Create Hits for numberClusters clusters, for each cluster we
// generate 2 hits in x and 2 hits in y. All hits are separated
// in time and space: y-coords are offset from x-coords by 1
// just for the heck of it. Hits within a plane are separated by
// interCoordTimeGap and hits between planes are separated by
// interPlaneTimeGap. Finally clusters are separated by (timegap + 1)
void createHits(uint32_t numberClusters) {
  uint8_t plane_x{0};
  uint8_t plane_y{1};
  uint16_t hit_adc{4000};
  uint64_t interCoordTimeGap = 5;
  uint64_t interPlaneTimeGap = 70; // max for this test to pass

  uint64_t t = 0;
  for (uint32_t i = 0; i < numberClusters; i++) {
    uint16_t coordStart = i % 32;
    // x-plane Hits
    builder.insert({t, coordStart    , hit_adc, plane_x});

    t += interCoordTimeGap;
    builder.insert({t, uint16_t(coordStart + 1), hit_adc, plane_x});

    // y-plane Hits
    t += interPlaneTimeGap;
    builder.insert({t, uint16_t(coordStart + 1), hit_adc, plane_y});

    t += interCoordTimeGap;
    builder.insert({t, uint16_t(coordStart + 2), hit_adc, plane_y});

    t+= timegap + 1;
  }
}


static void EventGenBM(benchmark::State &state) {
  uint32_t items = 0;
  double xsum = 0.0;
  double ysum = 0.0;
  uint32_t N = state.range(0) ;

  for (auto _ : state) {
    createHits(N);

    builder.flush();

    for (auto & e : builder.matcher.matched_events) {
      auto x = e.c1.coord_center();
      auto y = e.c2.coord_center();
      xsum += x;
      ysum += y;
    }
    items += N;
  }
  state.SetItemsProcessed(items);
  printf("xsum %f, ysum %f\n", xsum, ysum);
}

BENCHMARK(EventGenBM)->Range(16, 16<<10);

BENCHMARK_MAIN();
