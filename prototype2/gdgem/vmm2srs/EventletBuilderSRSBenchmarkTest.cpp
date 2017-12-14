/** Copyright (C) 2017 European Spallation Source ERIC */

#include <benchmark/benchmark.h>
#include <gdgem/vmm2srs/EventletBuilderSRS.h>
#include <gdgem/vmm2srs/SRSTestData.h>
#include <string>
#include <unistd.h>

BuilderSRS *builder;
Clusterer clusterer(30);
NMXHists hists;

static void Setup(__attribute__((unused)) benchmark::State &state) {
  SRSMappings geometry;
  SRSTime time;
  geometry.define_plane(0, {{1, 0}, {1, 1}, {1, 6}, {1, 7}});
  geometry.define_plane(1, {{1, 10}, {1, 11}, {1, 14}, {1, 15}});
  builder = new BuilderSRS(time, geometry, "", false, false);
}
BENCHMARK(Setup);

static void ParseData(benchmark::State &state) {
  uint64_t eventlets = 0;
  Setup(state);

  for (auto _ : state) {
    auto stats =
        builder->process_buffer((char *)srsdata_22_eventlets,
                                sizeof(srsdata_22_eventlets), clusterer, hists);
    eventlets += stats.valid_eventlets;
  }
  state.SetBytesProcessed(state.iterations() * sizeof(srsdata_22_eventlets));
  state.SetItemsProcessed(eventlets);
};
BENCHMARK(ParseData);

BENCHMARK_MAIN()
