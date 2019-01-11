/** Copyright (C) 2019 European Spallation Source ERIC */

#include <multiblade/clustering/EventBuilder.h>
#include <benchmark/benchmark.h>
#include <string>
#include <unistd.h>

Multiblade::EventBuilder builder;


#include "EventBuilderCommon.inc"


static void EventGenBM(benchmark::State &state) {
  uint32_t items = 0;
  double xsum = 0.0;
  double ysum = 0.0;
  uint32_t N = state.range(0) ;

  for (auto _ : state) {
    createHits(N, 2);

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

BENCHMARK(EventGenBM)->RangeMultiplier(2)->Range(64, 2<<12);

BENCHMARK_MAIN();
