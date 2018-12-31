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
#include <multigrid/generators/BuilderReadouts.h>
#include <multigrid/generators/ReaderReadouts.h>
#include <multigrid/MgConfig.h>
#include <test/TestBase.h>

using namespace Multigrid;

// \todo factor this out to some file under common
class SimpleHist1D {
public:
  std::vector<size_t> hist;
  void bin(size_t i) {
    if (hist.size() <= i)
      hist.resize(i + 1, 0);
    hist[i]++;
  }

  bool empty() const {
    return hist.empty();
  }

  std::string debug() const {
    std::stringstream ss;
    for (size_t i = 0; i < hist.size(); ++i)
      ss << "[" << i << "]=" << hist[i];
    return ss.str();
  }

  std::string visualize(bool non_empty_only = false) const {
    if (hist.empty())
      return {};

    size_t vmax{hist[0]};
    size_t start{0}, end{0};
    bool print{false};
    for (uint32_t i = 0; i < hist.size(); i++) {
      const auto &val = hist[i];
      vmax = std::max(vmax, val);
      if (val > 0) {
        end = i;
        if (!print) {
          start = i;
          print = true;
        }
      }
    }

    std::string largesti = fmt::format("{}", end);
    std::string pad = "{:<" + fmt::format("{}", largesti.size()) + "}";

    // \todo parametrize this
    size_t nstars{60};

    std::stringstream ss;
    for (size_t i = start; i <= end; i++) {
      auto val = hist[i];
      if (!non_empty_only || (val > 0))
        ss << fmt::format(pad, i) << ": "
           << fmt::format("{:<62}", std::string((nstars * val) / vmax, '*'))
           << val << "\n";
    }
    return ss.str();
  }
};

class ReductionTest : public TestBase {
protected:
  uint64_t ShortestPulsePeriod{std::numeric_limits<uint64_t>::max()};
  size_t ingested_hits{0};

  BuilderReadouts builder;
  Reduction reduction;

  virtual void SetUp() {
    load_config(TEST_DATA_PATH "Sequoia_mappings.json");
  }
  virtual void TearDown() {
  }

  void load_config(const std::string &jsonfile) {
    Multigrid::Config config(jsonfile);
    builder.digital_geometry = config.analyzer.mappings;
    //MESSAGE() << "Digital geometry: " << builder.digital_geometry.debug() << "\n";
  }

  void feed_file(const std::string &filename) {
    ReaderReadouts reader(filename);

    uint8_t buffer[9000];
    size_t readsz;

    while ((readsz = reader.read((char *) &buffer)) > 0) {
      builder.parse(Buffer<uint8_t>(buffer, readsz));
      ingested_hits += builder.ConvertedData.size();
      reduction.ingest(builder.ConvertedData);
      reduction.perform_clustering(false);
    }
    reduction.perform_clustering(true);
  }

  void inspect_pulse_data(bool verbose = false) {
    bool HavePulseTime{false};
    uint64_t RecentPulseTime{0};

    SimpleHist1D pulse_positive_diff, pulse_negative_diff;
    for (const auto &pt : reduction.pulse_times) {
      if (HavePulseTime) {
        if (pt >= RecentPulseTime) {
          pulse_positive_diff.bin(pt - RecentPulseTime);
          auto PulsePeriod = pt - RecentPulseTime;
          ShortestPulsePeriod = std::min(ShortestPulsePeriod, PulsePeriod);
        }
        if (pt < RecentPulseTime) {
          pulse_negative_diff.bin(RecentPulseTime - pt);
        }
      }
      RecentPulseTime = pt;
      HavePulseTime = true;
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

    SimpleHist1D event_positive_diff, event_negative_diff;
    SimpleHist1D wire_multiplicity, wire_span, grid_mltiplicity, grid_span, time_span;
    for (const auto &e : reduction.matcher.matched_events) {
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

  EXPECT_EQ(reduction.pulse_times.size(), 467);
  EXPECT_EQ(reduction.wire_clusters.stats_cluster_count, 164);
  EXPECT_EQ(reduction.grid_clusters.stats_cluster_count, 184);
  EXPECT_EQ(reduction.matcher.matched_events.size(), 182);

  inspect_pulse_data();
  inspect_event_data();
  EXPECT_EQ(ShortestPulsePeriod, 266662);
}

TEST_F(ReductionTest, t00033) {
  feed_file(TEST_DATA_PATH "readouts/154493");

  EXPECT_EQ(ingested_hits, 8724);
  EXPECT_EQ(reduction.stats_invalid_planes, 0);
  EXPECT_EQ(reduction.stats_time_seq_errors, 1);

  EXPECT_EQ(reduction.pulse_times.size(), 2555);
  EXPECT_EQ(reduction.wire_clusters.stats_cluster_count, 1737);
  EXPECT_EQ(reduction.grid_clusters.stats_cluster_count, 1934);
  EXPECT_EQ(reduction.matcher.matched_events.size(), 1821);

  inspect_pulse_data();
  inspect_event_data();
  EXPECT_EQ(ShortestPulsePeriod, 266662);
}

TEST_F(ReductionTest, t00311) {
  feed_file(TEST_DATA_PATH "readouts/154492");

  EXPECT_EQ(ingested_hits, 84352);
  EXPECT_EQ(reduction.stats_invalid_planes, 0);
  EXPECT_EQ(reduction.stats_time_seq_errors, 35);

  EXPECT_EQ(reduction.pulse_times.size(), 975);
  EXPECT_EQ(reduction.wire_clusters.stats_cluster_count, 23369);
  EXPECT_EQ(reduction.grid_clusters.stats_cluster_count, 26086);
  EXPECT_EQ(reduction.matcher.matched_events.size(), 20461);

  inspect_pulse_data();
  inspect_event_data();
  EXPECT_EQ(ShortestPulsePeriod, 0);
}

TEST_F(ReductionTest, t03710) {
  feed_file(TEST_DATA_PATH "readouts/154478");

  EXPECT_EQ(ingested_hits, 948716);
  EXPECT_EQ(reduction.stats_invalid_planes, 0);
  EXPECT_EQ(reduction.stats_time_seq_errors, 35);

  EXPECT_EQ(reduction.pulse_times.size(), 312);
  EXPECT_EQ(reduction.wire_clusters.stats_cluster_count, 21130);
  EXPECT_EQ(reduction.grid_clusters.stats_cluster_count, 20926);
  EXPECT_EQ(reduction.matcher.matched_events.size(), 19324);

  inspect_pulse_data();
  inspect_event_data();
  EXPECT_EQ(ShortestPulsePeriod, 266662);
}

TEST_F(ReductionTest, t10392) {
  feed_file(TEST_DATA_PATH "readouts/154484");

  EXPECT_EQ(ingested_hits, 2656636);
  EXPECT_EQ(reduction.stats_invalid_planes, 0);
  EXPECT_EQ(reduction.stats_time_seq_errors, 1);

  EXPECT_EQ(reduction.pulse_times.size(), 300);
  EXPECT_EQ(reduction.wire_clusters.stats_cluster_count, 53924);
  EXPECT_EQ(reduction.grid_clusters.stats_cluster_count, 56093);
  EXPECT_EQ(reduction.matcher.matched_events.size(), 48574);

  inspect_pulse_data();
  inspect_event_data(true);
  EXPECT_EQ(ShortestPulsePeriod, 266662);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
