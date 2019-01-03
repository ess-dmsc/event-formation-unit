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
//      busses, indicative of coincidence across busses.
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
//      plane mappings), which will not be needed in the final VMM-based setup, so
//      we should not bother mitigating this. Simply reject those events.


#include <multigrid/reduction/Reduction.h>
#include <multigrid/mesytec/BuilderReadouts.h>
#include <multigrid/generators/ReaderReadouts.h>
#include <multigrid/Config.h>
#include <common/DynamicHist.h>
#include <test/TestBase.h>

using namespace Multigrid;

class ReductionTest : public TestBase {
protected:
  Multigrid::Config config;

  uint64_t ShortestPulsePeriod{std::numeric_limits<uint64_t>::max()};
  size_t ingested_hits{0};
  size_t pulse_times{0};
  size_t neutron_events{0};

  Reduction reduction;

  virtual void SetUp() {
    load_config(TEST_DATA_PATH "Sequoia_mappings.json");
  }
  virtual void TearDown() {
  }

  void load_config(const std::string &jsonfile) {
    config = Multigrid::Config(jsonfile);
    config.builder = std::make_shared<BuilderReadouts>(config.analyzer.mappings);
    //MESSAGE() << "Digital geometry: " << config.builder->digital_geometry.debug() << "\n";
  }

  void feed_file(const std::string &filename) {
    ReaderReadouts reader(filename);

    uint8_t buffer[9000];
    size_t readsz;

    while ((readsz = reader.read((char *) &buffer)) > 0) {
      config.builder->parse(Buffer<uint8_t>(buffer, readsz));
      ingested_hits += config.builder->ConvertedData.size();
      reduction.ingest(config.builder->ConvertedData);
      reduction.perform_clustering(false);
    }
    reduction.perform_clustering(true);
  }

  void inspect_pulse_data(bool verbose = false) {
    bool HavePulseTime{false};
    uint64_t RecentPulseTime{0};

    DynamicHist pulse_positive_diff, pulse_negative_diff;
    for (const auto &e : reduction.matcher.matched_events) {
      if (e.plane1() != AbstractBuilder::external_trigger_plane)
        continue;

      if (HavePulseTime) {
        if (e.time_start() >= RecentPulseTime) {
          pulse_positive_diff.bin(e.time_start() - RecentPulseTime);
          auto PulsePeriod = e.time_start() - RecentPulseTime;
          ShortestPulsePeriod = std::min(ShortestPulsePeriod, PulsePeriod);
        }
        if (e.time_start() < RecentPulseTime) {
          pulse_negative_diff.bin(RecentPulseTime - e.time_start());
        }
      }
      RecentPulseTime = e.time_start();
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
    DynamicHist wire_multiplicity, wire_span, grid_mltiplicity, grid_span, time_span;
    for (const auto &e : reduction.matcher.matched_events) {
      if (e.plane1() == AbstractBuilder::external_trigger_plane)
        continue;

      neutron_events++;

      time_span.bin(e.time_span());
      wire_multiplicity.bin(e.c1.hit_count());
      wire_span.bin(e.c1.coord_span());
      grid_mltiplicity.bin(e.c2.hit_count());
      grid_span.bin(e.c2.coord_span());

      if (HaveTime) {
        auto t = e.time_start();
        if (t >= RecentTime) {
          event_positive_diff.bin(t - RecentTime);
        }
        if (t < RecentTime) {
          event_negative_diff.bin(RecentTime - t);
        }
      }
      RecentTime = e.time_start();
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
      MESSAGE() << "Wire multiplicity:\n" << wire_multiplicity.visualize(true) << "\n";
      MESSAGE() << "Wire span:\n" << wire_span.visualize(true) << "\n";
      MESSAGE() << "Grid multiplicity:\n" << grid_mltiplicity.visualize(true) << "\n";
      MESSAGE() << "Grid span:\n" << grid_span.visualize(true) << "\n";
    }
    //      if ((i > 80000) && (i < 85000)) {
//        MESSAGE() << h.debug() << "\n";
//      }

  }

};

TEST_F(ReductionTest, t00004) {
  feed_file(TEST_DATA_PATH "readouts/154482");

  EXPECT_EQ(ingested_hits, 1088);
  EXPECT_EQ(reduction.stats_invalid_planes, 0);
  EXPECT_EQ(reduction.stats_time_seq_errors, 0);

  EXPECT_EQ(reduction.stats_wire_clusters, 164);
  EXPECT_EQ(reduction.stats_grid_clusters, 184);

  inspect_pulse_data();
  inspect_event_data();
  EXPECT_EQ(pulse_times, 467);
  EXPECT_EQ(neutron_events, 182);
  EXPECT_EQ(ShortestPulsePeriod, 266662);
}

TEST_F(ReductionTest, t00033) {
  feed_file(TEST_DATA_PATH "readouts/154493");

  EXPECT_EQ(ingested_hits, 8724);
  EXPECT_EQ(reduction.stats_invalid_planes, 0);
  EXPECT_EQ(reduction.stats_time_seq_errors, 1);

  EXPECT_EQ(reduction.stats_wire_clusters, 1737);
  EXPECT_EQ(reduction.stats_grid_clusters, 1934);

  inspect_pulse_data();
  inspect_event_data();
  EXPECT_EQ(pulse_times, 2555);
  EXPECT_EQ(neutron_events, 1822);
  EXPECT_EQ(ShortestPulsePeriod, 266662);
}

TEST_F(ReductionTest, t00311) {
  feed_file(TEST_DATA_PATH "readouts/154492");

  EXPECT_EQ(ingested_hits, 84232);
  EXPECT_EQ(reduction.stats_invalid_planes, 0);
  EXPECT_EQ(reduction.stats_time_seq_errors, 34);

  EXPECT_EQ(reduction.stats_wire_clusters, 23368);
  EXPECT_EQ(reduction.stats_grid_clusters, 26085);

  inspect_pulse_data();
  inspect_event_data();
  EXPECT_EQ(pulse_times, 975);
  EXPECT_EQ(neutron_events, 20460);
  EXPECT_EQ(ShortestPulsePeriod, 0);
}

TEST_F(ReductionTest, t03710) {
  feed_file(TEST_DATA_PATH "readouts/154478");

  EXPECT_EQ(ingested_hits, 55666);
  EXPECT_EQ(reduction.stats_invalid_planes, 0);
  EXPECT_EQ(reduction.stats_time_seq_errors, 0);

  EXPECT_EQ(reduction.stats_wire_clusters, 16755);
  EXPECT_EQ(reduction.stats_grid_clusters, 16370);

  inspect_pulse_data();
  inspect_event_data();
  EXPECT_EQ(pulse_times, 312);
  EXPECT_EQ(neutron_events, 14341);
  EXPECT_EQ(ShortestPulsePeriod, 266662);
}

TEST_F(ReductionTest, t10392) {
  feed_file(TEST_DATA_PATH "readouts/154484");

  EXPECT_EQ(ingested_hits, 178941);
  EXPECT_EQ(reduction.stats_invalid_planes, 0);
  EXPECT_EQ(reduction.stats_time_seq_errors, 1);

  EXPECT_EQ(reduction.stats_wire_clusters, 51947);
  EXPECT_EQ(reduction.stats_grid_clusters, 51344);

  inspect_pulse_data();
  inspect_event_data();
  EXPECT_EQ(pulse_times, 300);
  EXPECT_EQ(neutron_events, 41813);
  EXPECT_EQ(ShortestPulsePeriod, 266662);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
