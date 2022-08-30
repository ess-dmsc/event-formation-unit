/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

// OBSERVATIONS
//
// Pulse times (external triggers):
//    Occasionally repeated (delta=0), though no decreases are observed.
//
// Events:
//    Time difference is occasionally (rarely) negative. This appears to happen
//      because of leftover data from previous run.
//    Time span is always 1, as prescribed in clustering setup.
//    Wire multiplicity is close to 1, except when it is 80,
//      which happens in cases of all wires firing (bus glitch).
//    Wire span shows some expected variation. It is often 80, again indicative
//      of the "bus glitch". It is occasionally above 80, i.e. spanning multiple
//      buses, indicative of coincidence across buses.
//    Grid multiplicity is generally in the range of 1-7. It is often 40, which
//      is indicative of "bus glitch".
//    Grid span - same as wire span, except 40 in case of glitch
//
// CONSEQUENCES FOR PIPELINE
//    Anticipate repeated pulse times.
//    Discard out-of-sequence events. Expect to discard a lot at the beginning
//      of run.
//    Filter events with higher multiplicities and spans in either dimension.
//    Clustering across buses is a "mistake" and ideally should be avoided, but
//      this likely requires more complexity (multiple clusterers and additional
//      plane mappings), which will not be needed in the final VMM-based setup,
//      so we should not bother mitigating this. Simply reject those events.

#include <common/monitor/DynamicHist.h>
#include <common/testutils/TestBase.h>
#include <multigrid/Config.h>
#include <multigrid/generators/ReaderReadouts.h>
#include <multigrid/mesytec/BuilderReadouts.h>
#include <multigrid/reduction/Reduction.h>

using namespace Multigrid;

class ReductionTest : public TestBase {
protected:
  Multigrid::Config config;

  uint64_t ShortestPulsePeriod{std::numeric_limits<uint64_t>::max()};
  size_t ingested_hits{0};
  size_t pulse_times{0};
  size_t good_events{0};

  void SetUp() override {
    load_config(TEST_DATA_PATH "Sequoia_mappings2.json");
  }
  void TearDown() override {}

  void load_config(const std::string &jsonfile) {
    config = Multigrid::Config(jsonfile);
    //    MESSAGE() << "Config:\n" << config.debug() << "\n";
  }

  void feed_file(const std::string &filename) {
    ReaderReadouts reader(filename);

    uint8_t buffer[9000];
    size_t readsz;

    while ((readsz = reader.read((char *)&buffer)) > 0) {
      config.builder->parse(Buffer<uint8_t>(buffer, readsz));
      ingested_hits += config.builder->ConvertedData.size();
      config.reduction.ingest(config.builder->ConvertedData);
      config.builder->ConvertedData.clear();

      config.reduction.process_queues(false);
    }
    config.reduction.process_queues(true);

    //    MESSAGE() << "Status:\n" << config.reduction.status("", false);
  }

  void inspect_pulse_data(bool verbose = false) {
    bool HavePulseTime{false};
    uint64_t RecentPulseTime{0};

    DynamicHist pulse_positive_diff, pulse_negative_diff;
    for (const auto &e : config.reduction.out_queue) {
      if (e.pixel_id != 0) {
        continue;
      }

      if (HavePulseTime) {
        if (e.time >= RecentPulseTime) {
          pulse_positive_diff.bin(e.time - RecentPulseTime);
          auto PulsePeriod = e.time - RecentPulseTime;
          ShortestPulsePeriod = std::min(ShortestPulsePeriod, PulsePeriod);
        }
        if (e.time < RecentPulseTime) {
          pulse_negative_diff.bin(RecentPulseTime - e.time);
        }
      }
      RecentPulseTime = e.time;
      HavePulseTime = true;
      pulse_times++;
    }

    if (verbose) {
      MESSAGE() << "Pulse positive time difference:\n"
                << pulse_positive_diff.visualize(true) << "\n";
      if (!pulse_negative_diff.empty()) {
        MESSAGE() << "Pulse negative time difference:\n"
                  << pulse_negative_diff.visualize(true) << "\n";
      }
    }
  }

  void inspect_event_data(bool verbose = false) {

    bool HaveTime{false};
    uint64_t RecentTime{0};

    DynamicHist event_positive_diff, event_negative_diff;
    DynamicHist wire_multiplicity, wire_span, grid_mltiplicity, grid_span,
        time_span;
    for (const auto &e : config.reduction.out_queue) {
      if (e.pixel_id == 0) {
        continue;
      }

      good_events++;

      //      time_span.bin(e.time_span());
      //      wire_multiplicity.bin(e.cluster1.hit_count());
      //      wire_span.bin(e.cluster1.coord_span());
      //      grid_mltiplicity.bin(e.cluster2.hit_count());
      //      grid_span.bin(e.cluster2.coord_span());

      if (HaveTime) {
        auto t = e.time;
        if (t >= RecentTime) {
          event_positive_diff.bin(t - RecentTime);
        }
        if (t < RecentTime) {
          event_negative_diff.bin(RecentTime - t);
        }
      }
      RecentTime = e.time;
      HaveTime = true;
    }

    if (verbose) {
      //      MESSAGE() << "Event positive time difference:\n"
      //                << event_positive_diff.visualize(true) << "\n";
      if (!event_negative_diff.empty()) {
        MESSAGE() << "Event negative time difference:\n"
                  << event_negative_diff.visualize(true) << "\n";
      }
      MESSAGE() << "Event time span:\n" << time_span.visualize(true) << "\n";
      MESSAGE() << "Wire multiplicity:\n"
                << wire_multiplicity.visualize(true) << "\n";
      MESSAGE() << "Wire span:\n" << wire_span.visualize(true) << "\n";
      MESSAGE() << "Grid multiplicity:\n"
                << grid_mltiplicity.visualize(true) << "\n";
      MESSAGE() << "Grid span:\n" << grid_span.visualize(true) << "\n";
    }
    //      if ((i > 80000) && (i < 85000)) {
    //        MESSAGE() << h.to_string() << "\n";
    //      }
  }
};

TEST_F(ReductionTest, t00004) {
  feed_file(TEST_DATA_PATH "readouts/154482");

  EXPECT_EQ(ingested_hits, 1088);
  EXPECT_EQ(config.reduction.stats.invalid_planes, 0);
  EXPECT_EQ(config.reduction.stats.time_seq_errors, 0);

  EXPECT_EQ(config.reduction.stats.wire_clusters, 164);
  EXPECT_EQ(config.reduction.stats.grid_clusters, 184);
  EXPECT_EQ(config.reduction.stats.events_total, 196);
  EXPECT_EQ(config.reduction.stats.events_multiplicity_rejects, 13);
  EXPECT_EQ(config.reduction.stats.hits_used, 305);
  EXPECT_EQ(config.reduction.stats.events_bad, 62);
  EXPECT_EQ(config.reduction.stats.events_geometry_err, 0);

  inspect_pulse_data();
  inspect_event_data();
  EXPECT_EQ(pulse_times, 467);
  EXPECT_EQ(good_events, 121);
  EXPECT_EQ(good_events + config.reduction.stats.events_multiplicity_rejects +
                config.reduction.stats.events_bad,
            config.reduction.stats.events_total);
  EXPECT_EQ(ShortestPulsePeriod, 266662);
}

TEST_F(ReductionTest, t00033) {
  feed_file(TEST_DATA_PATH "readouts/154493");

  EXPECT_EQ(ingested_hits, 8724);
  EXPECT_EQ(config.reduction.stats.invalid_planes, 0);
  EXPECT_EQ(config.reduction.stats.time_seq_errors, 2);

  EXPECT_EQ(config.reduction.stats.wire_clusters, 1737);
  EXPECT_EQ(config.reduction.stats.grid_clusters, 1934);
  EXPECT_EQ(config.reduction.stats.events_total, 1932);
  EXPECT_EQ(config.reduction.stats.events_multiplicity_rejects, 117);
  EXPECT_EQ(config.reduction.stats.hits_used, 3197);
  EXPECT_EQ(config.reduction.stats.events_bad, 438);
  EXPECT_EQ(config.reduction.stats.events_geometry_err, 0);

  inspect_pulse_data();
  inspect_event_data();
  EXPECT_EQ(pulse_times, 2555);
  EXPECT_EQ(good_events, 1377);
  EXPECT_EQ(good_events + config.reduction.stats.events_multiplicity_rejects +
                config.reduction.stats.events_bad,
            config.reduction.stats.events_total);
  EXPECT_EQ(ShortestPulsePeriod, 266662);
}

TEST_F(ReductionTest, t00311) {
  feed_file(TEST_DATA_PATH "readouts/154492");

  EXPECT_EQ(ingested_hits, 84232);
  EXPECT_EQ(config.reduction.stats.invalid_planes, 0);
  EXPECT_EQ(config.reduction.stats.time_seq_errors, 68);

  EXPECT_EQ(config.reduction.stats.wire_clusters, 23334);
  EXPECT_EQ(config.reduction.stats.grid_clusters, 26051);

  EXPECT_EQ(config.reduction.stats.events_total, 20876);
  EXPECT_EQ(config.reduction.stats.events_multiplicity_rejects, 2763);
  EXPECT_EQ(config.reduction.stats.hits_used, 36144);
  EXPECT_EQ(config.reduction.stats.events_bad, 127);
  EXPECT_EQ(config.reduction.stats.events_geometry_err, 0);

  inspect_pulse_data();
  inspect_event_data();
  EXPECT_EQ(pulse_times, 975);
  EXPECT_EQ(good_events, 17986);
  EXPECT_EQ(good_events + config.reduction.stats.events_multiplicity_rejects +
                config.reduction.stats.events_bad,
            config.reduction.stats.events_total);
  EXPECT_EQ(ShortestPulsePeriod, 0);
}

TEST_F(ReductionTest, t03710) {
  feed_file(TEST_DATA_PATH "readouts/154478");

  EXPECT_EQ(ingested_hits, 55666);
  EXPECT_EQ(config.reduction.stats.invalid_planes, 0);
  EXPECT_EQ(config.reduction.stats.time_seq_errors, 0);

  EXPECT_EQ(config.reduction.stats.wire_clusters, 16755);
  EXPECT_EQ(config.reduction.stats.grid_clusters, 16370);

  EXPECT_EQ(config.reduction.stats.events_total, 15879);
  EXPECT_EQ(config.reduction.stats.events_multiplicity_rejects, 1959);
  EXPECT_EQ(config.reduction.stats.hits_used, 25085);
  EXPECT_EQ(config.reduction.stats.events_bad, 2768);
  EXPECT_EQ(config.reduction.stats.events_geometry_err, 0);

  inspect_pulse_data();
  inspect_event_data();
  EXPECT_EQ(pulse_times, 312);
  EXPECT_EQ(good_events, 11152);
  EXPECT_EQ(good_events + config.reduction.stats.events_multiplicity_rejects +
                config.reduction.stats.events_bad,
            config.reduction.stats.events_total);
  EXPECT_EQ(ShortestPulsePeriod, 266662);
}

TEST_F(ReductionTest, t10392) {
  feed_file(TEST_DATA_PATH "readouts/154484");

  EXPECT_EQ(ingested_hits, 178941);
  EXPECT_EQ(config.reduction.stats.invalid_planes, 0);
  EXPECT_EQ(config.reduction.stats.time_seq_errors, 1);

  EXPECT_EQ(config.reduction.stats.wire_clusters, 51950);
  EXPECT_EQ(config.reduction.stats.grid_clusters, 51344);

  EXPECT_EQ(config.reduction.stats.events_total, 43050);
  EXPECT_EQ(config.reduction.stats.events_multiplicity_rejects, 8971);
  EXPECT_EQ(config.reduction.stats.hits_used, 67455);
  EXPECT_EQ(config.reduction.stats.events_bad, 740);
  EXPECT_EQ(config.reduction.stats.events_geometry_err, 0);

  inspect_pulse_data();
  inspect_event_data();
  EXPECT_EQ(pulse_times, 300);
  EXPECT_EQ(good_events, 33339);
  EXPECT_EQ(good_events + config.reduction.stats.events_multiplicity_rejects +
                config.reduction.stats.events_bad,
            config.reduction.stats.events_total);
  EXPECT_EQ(ShortestPulsePeriod, 266662);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
