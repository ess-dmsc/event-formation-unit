/** Copyright (C) 2017 European Spallation Source ERIC */

#include <benchmark/benchmark.h>
#include <gdgem/clustering/DoroClusterer.h>
#include <gdgem/vmm2/BuilderVMM2.h>
#include <gdgem/vmm2/ParserVMM2TestData.h>
#include <gdgem/NMXConfig.h>
#include <string>
#include <unistd.h>

std::shared_ptr<AbstractBuilder> builder{nullptr};

static void Setup(__attribute__((unused)) benchmark::State &state) {
  NMXConfig nmx_opts;

  auto clusx = std::make_shared<DoroClusterer>(nmx_opts.clusterer_x.max_time_gap,
                                            nmx_opts.clusterer_x.max_strip_gap,
                                            nmx_opts.clusterer_x.min_cluster_size);
  auto clusy = std::make_shared<DoroClusterer>(nmx_opts.clusterer_y.max_time_gap,
                                            nmx_opts.clusterer_y.max_strip_gap,
                                            nmx_opts.clusterer_y.min_cluster_size);

  builder = std::make_shared<BuilderVMM2>(
    nmx_opts.time_config, nmx_opts.srs_mappings, clusx, clusy,
    nmx_opts.clusterer_x.hit_adc_threshold, nmx_opts.clusterer_x.max_time_gap,
    nmx_opts.clusterer_y.hit_adc_threshold, nmx_opts.clusterer_y.max_time_gap,
    nmx_opts.dump_directory, nmx_opts.dump_csv, nmx_opts.dump_h5);
}
BENCHMARK(Setup);

static void ParseData(benchmark::State &state) {
  uint64_t hits = 0;
  Setup(state);

  for (auto _ : state) {
    auto stats =
        builder->process_buffer((char *)srsdata_22_hits, sizeof(srsdata_22_hits));
    hits += stats.valid_hits;
  }
  state.SetBytesProcessed(state.iterations() * sizeof(srsdata_22_hits));
  state.SetItemsProcessed(hits);
};
BENCHMARK(ParseData);

BENCHMARK_MAIN();
