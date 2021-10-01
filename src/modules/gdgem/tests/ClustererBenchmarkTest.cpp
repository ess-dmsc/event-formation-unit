/** Copyright (C) 2018 European Spallation Source ERIC */

#include <benchmark/benchmark.h>
#include <gdgem/NMXConfig.h>
#include <common/testutils/TestBase.h>

#include <gdgem/nmx/Readout.h>
#include <common/reduction/clustering/GapClusterer.h>
#include <common/reduction/matching/GapMatcher.h>

using namespace Gem;

class HitSorter {
public:
  HitSorter(SRSTime time, SRSMappings chips) :
      pTime(time), pChips(chips)
  {}

  void insert(const Readout &readout) {
    buffer.push_back(Hit());
    auto &e = buffer.back();
    e.plane = pChips.get_plane(readout);
    e.weight = readout.adc;
    e.coordinate = pChips.get_strip(readout);
    e.time = readout.srs_timestamp + static_cast<uint64_t>(readout.chiptime);
    if (readout.srs_timestamp < prev_srs_time)
      srs_overflows++;
    prev_srs_time = readout.srs_timestamp;
    if (readout.chiptime < 0)
      negative_chip_times++;
  }

  void flush() {
    sort_chronologically(buffer);
    if (clusterer)
      clusterer->cluster(buffer);
    buffer.clear();
    if (clusterer)
      clusterer->flush();
  }

  std::shared_ptr<AbstractClusterer> clusterer;

  HitVector buffer;
  uint64_t prev_srs_time {0};
  size_t srs_overflows{0};
  size_t negative_chip_times{0};

private:
  SRSTime pTime;
  SRSMappings pChips;
};

static void Doit(benchmark::State &state) {
	std::string DataPath = TEST_DATA_PATH;
  auto opts = NMXConfig(DataPath + "/config.json", "");

    HitSorter sorter_x(opts.time_config, opts.srs_mappings);
  HitSorter sorter_y(opts.time_config, opts.srs_mappings);

  sorter_x.clusterer =
      std::make_shared<GapClusterer>(opts.clusterer_x.max_time_gap,
                                     opts.clusterer_x.max_strip_gap);
  sorter_y.clusterer =
      std::make_shared<GapClusterer>(opts.clusterer_y.max_time_gap,
                                     opts.clusterer_y.max_strip_gap);

  GapMatcher matcher (opts.time_config.acquisition_window()*5, 0, 1);
  matcher.set_minimum_time_gap(opts.matcher_max_delta_time);

  std::vector<Readout> readouts;
  ReadoutFile::read(DataPath + "/readouts/a10000", readouts);


  uint32_t items = 0;

	for (auto _ : state) {
		for (const auto& readout : readouts) {
      auto plane = opts.srs_mappings.get_plane(readout);
      if (plane == 0) {
        sorter_x.insert(readout);
      }
      if (plane == 1) {
        sorter_y.insert(readout);
      }
		}

    sorter_x.flush();
    sorter_y.flush();

    matcher.insert(0, sorter_x.clusterer->clusters);
    matcher.insert(1, sorter_y.clusterer->clusters);
    matcher.match(true);
    matcher.matched_events.clear();

		// number of readouts
		items += readouts.size();

		// number of clusters from matcher?
	}
	// state.SetComplexityN(state.range(0));
	//state.SetBytesProcessed(state.iterations() * state.range(0));
	state.SetItemsProcessed(items);
}

BENCHMARK (Doit);

BENCHMARK_MAIN();
