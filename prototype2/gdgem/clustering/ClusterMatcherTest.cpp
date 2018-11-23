/** Copyright (C) 2017 European Spallation Source ERIC */

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <gdgem/clustering/HitSorter.h>
#include <gdgem/clustering/DoroClusterer.h>
#include <gdgem/clustering/ClusterMatcher.h>
#include <test/TestBase.h>
#include <functional>
#include <gdgem/NMXConfig.h>

#include <gdgem/nmx/Readout.h>

#define UNUSED __attribute__((unused))

using namespace Gem;

class ClusterMatcherTest : public TestBase {
protected:
  NMXConfig opts;
  std::string DataPath;
  std::vector<Readout> readouts;

  std::shared_ptr<ClusterMatcher> matcher;
  std::shared_ptr<HitSorter> sorter_x;
  std::shared_ptr<HitSorter> sorter_y;

  virtual void SetUp() {
    DataPath = TEST_DATA_PATH;
    opts = NMXConfig(DataPath + "/config.json", "");

    matcher = std::make_shared<ClusterMatcher>(opts.matcher_max_delta_time);

    sorter_x = std::make_shared<HitSorter>(opts.time_config, opts.srs_mappings,
                                           opts.clusterer_x.hit_adc_threshold,
                                           opts.clusterer_x.max_time_gap);
    sorter_y = std::make_shared<HitSorter>(opts.time_config, opts.srs_mappings,
                                           opts.clusterer_y.hit_adc_threshold,
                                           opts.clusterer_y.max_time_gap);

    sorter_x->clusterer = std::make_shared<DoroClusterer>(opts.clusterer_x.max_time_gap,
                                                          opts.clusterer_x.max_strip_gap,
                                                          opts.clusterer_x.min_cluster_size);
    sorter_y->clusterer = std::make_shared<DoroClusterer>(opts.clusterer_y.max_time_gap,
                                                          opts.clusterer_y.max_strip_gap,
                                                          opts.clusterer_y.min_cluster_size);
  }

  virtual void TearDown() {
  }

  void store_hit(const Readout &readout) {
    uint8_t plane = opts.srs_mappings.get_plane(readout);
    EXPECT_LT(plane, 2) << "fec:" << int(readout.fec)
                        << " chip:" << int(readout.chip_id) << "\n";
    if (plane == 0) {
      sorter_x->insert(readout);
    }
    if (plane == 1) {
      sorter_y->insert(readout);
    }
  }

  Cluster mock_cluster(uint8_t plane, uint16_t strip_start, uint16_t strip_end,
                       double time_start, double time_end) {
    Cluster ret;
    Hit e;
    e.plane_id = plane;
    e.adc = 1;
    double time_step = (time_end - time_start) / 10.0;
    for (e.time = time_start; e.time <= time_end; e.time += time_step)
      for (e.strip = strip_start; e.strip <= strip_end; ++e.strip)
        ret.insert_hit(e);
    e.time = time_end;
    ret.insert_hit(e);
    return ret;
  }

};

TEST_F(ClusterMatcherTest, OneX) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0, 10, 0, 200));
  matcher->match_end(true);
  ASSERT_EQ(matcher->stats_cluster_count, 1);
  ASSERT_EQ(matcher->matched_clusters.size(), 1);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 200);
  EXPECT_EQ(matcher->matched_clusters.front().x.hits.size(), 122);
  EXPECT_EQ(matcher->matched_clusters.front().y.hits.size(), 0);
}

TEST_F(ClusterMatcherTest, OneY) {
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0, 10, 0, 200));
  matcher->match_end(true);
  ASSERT_EQ(matcher->stats_cluster_count, 1);
  ASSERT_EQ(matcher->matched_clusters.size(), 1);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 200);
  EXPECT_EQ(matcher->matched_clusters.front().x.hits.size(), 0);
  EXPECT_EQ(matcher->matched_clusters.front().y.hits.size(), 122);
}

TEST_F(ClusterMatcherTest, TwoX) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0, 10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0, 10, 700, 900));
  matcher->match_end(true);
  ASSERT_EQ(matcher->stats_cluster_count, 2);
  ASSERT_EQ(matcher->matched_clusters.size(), 2);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 200);
  EXPECT_EQ(matcher->matched_clusters.back().time_span(), 200);
  EXPECT_EQ(matcher->matched_clusters.front().x.hits.size(), 122);
  EXPECT_EQ(matcher->matched_clusters.front().y.hits.size(), 0);
  EXPECT_EQ(matcher->matched_clusters.back().x.hits.size(), 122);
  EXPECT_EQ(matcher->matched_clusters.back().y.hits.size(), 0);
}

TEST_F(ClusterMatcherTest, TwoY) {
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0, 10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0, 10, 700, 900));
  matcher->match_end(true);
  ASSERT_EQ(matcher->stats_cluster_count, 2);
  ASSERT_EQ(matcher->matched_clusters.size(), 2);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 200);
  EXPECT_EQ(matcher->matched_clusters.back().time_span(), 200);
  EXPECT_EQ(matcher->matched_clusters.front().x.hits.size(), 0);
  EXPECT_EQ(matcher->matched_clusters.front().y.hits.size(), 122);
  EXPECT_EQ(matcher->matched_clusters.back().x.hits.size(), 0);
  EXPECT_EQ(matcher->matched_clusters.back().y.hits.size(), 122);
}

TEST_F(ClusterMatcherTest, OneXOneY) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0, 10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0, 10, 700, 900));
  matcher->match_end(true);
  ASSERT_EQ(matcher->stats_cluster_count, 2);
  ASSERT_EQ(matcher->matched_clusters.size(), 2);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 200);
  EXPECT_EQ(matcher->matched_clusters.back().time_span(), 200);
  EXPECT_EQ(matcher->matched_clusters.front().x.hits.size(), 122);
  EXPECT_EQ(matcher->matched_clusters.front().y.hits.size(), 0);
  EXPECT_EQ(matcher->matched_clusters.back().x.hits.size(), 0);
  EXPECT_EQ(matcher->matched_clusters.back().y.hits.size(), 122);
}

TEST_F(ClusterMatcherTest, OneXY) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0, 10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0, 10, 0, 200));
  matcher->match_end(true);
  ASSERT_EQ(matcher->stats_cluster_count, 1);
  ASSERT_EQ(matcher->matched_clusters.size(), 1);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 200);
  EXPECT_EQ(matcher->matched_clusters.front().x.hits.size(), 122);
  EXPECT_EQ(matcher->matched_clusters.front().y.hits.size(), 122);
}

TEST_F(ClusterMatcherTest, TwoXY) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0, 10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0, 10, 1, 300));
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0, 10, 700, 900));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0, 10, 750, 950));
  matcher->match_end(true);
  ASSERT_EQ(matcher->stats_cluster_count, 2);
  ASSERT_EQ(matcher->matched_clusters.size(), 2);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 300);
  EXPECT_EQ(matcher->matched_clusters.back().time_span(), 250);
  EXPECT_EQ(matcher->matched_clusters.front().x.hits.size(), 122);
  EXPECT_EQ(matcher->matched_clusters.front().y.hits.size(), 122);
  EXPECT_EQ(matcher->matched_clusters.back().x.hits.size(), 122);
  EXPECT_EQ(matcher->matched_clusters.back().y.hits.size(), 122);
}

TEST_F(ClusterMatcherTest, JustIntside) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0, 10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0, 10, 200, 400));
  matcher->match_end(true);
  ASSERT_EQ(matcher->matched_clusters.size(), 1);
}

TEST_F(ClusterMatcherTest, JustOutside) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0, 10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0, 10, 200, 701));
  matcher->match_end(true);
  ASSERT_EQ(matcher->matched_clusters.size(), 2);
}

TEST_F(ClusterMatcherTest, DontForce) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0, 10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0, 10, 200, 401));
  matcher->match_end(false);
  ASSERT_EQ(matcher->matched_clusters.size(), 0);

  matcher->unmatched_clusters.push_back(mock_cluster(1, 0, 10, 800, 1000));
  matcher->match_end(false);
  ASSERT_EQ(matcher->matched_clusters.size(), 0);

  matcher->unmatched_clusters.push_back(mock_cluster(1, 0, 10, 900, 1000));
  matcher->match_end(false);
  ASSERT_EQ(matcher->matched_clusters.size(), 0);
}

// \todo push events in various buffer sizes and produce same results

TEST_F(ClusterMatcherTest, a1) {
  ReadoutFile::read(DataPath + "/readouts/a00001", readouts);
  EXPECT_EQ(readouts.size(), 144);

  uint64_t bonus = 0;
  uint64_t old = 0;
  for (auto readout : readouts) {
    if (readout.srs_timestamp < old)
      bonus++;
    old = readout.srs_timestamp;
    /// \todo this hack should not be necessary!
    readout.srs_timestamp += (bonus << 42);
    store_hit(readout);
  }

  /// \todo I don't trust these results anymore, please validate
  EXPECT_EQ(sorter_x->clusterer->stats_cluster_count, 16);
  EXPECT_EQ(sorter_y->clusterer->stats_cluster_count, 0);
  matcher->merge(0, sorter_x->clusterer->clusters);
  matcher->merge(1, sorter_y->clusterer->clusters);
  matcher->match_end(false);
  EXPECT_EQ(matcher->stats_cluster_count, 0);

  sorter_x->flush();
  sorter_y->flush();
  EXPECT_EQ(sorter_x->clusterer->stats_cluster_count, 20);
  EXPECT_EQ(sorter_y->clusterer->stats_cluster_count, 0);
  matcher->merge(0, sorter_x->clusterer->clusters);
  matcher->merge(1, sorter_y->clusterer->clusters);
  matcher->match_end(true);
  EXPECT_EQ(matcher->stats_cluster_count, 14);
}

TEST_F(ClusterMatcherTest, a1_identical) {
  ReadoutFile::read(DataPath + "/readouts/a00001", readouts);
  EXPECT_EQ(readouts.size(), 144);

  uint64_t bonus = 0;
  uint64_t old = 0;
  for (auto readout : readouts) {
    if (readout.srs_timestamp < old)
      bonus++;
    old = readout.srs_timestamp;
    /// \todo this hack should not be necessary!
    readout.srs_timestamp += (bonus << 42);
    sorter_y->insert(readout);
    sorter_x->insert(readout);
  }

  /// \todo I don't trust these results anymore, please validate
  EXPECT_EQ(sorter_x->clusterer->stats_cluster_count, 16);
  EXPECT_EQ(sorter_y->clusterer->stats_cluster_count, 16);
  matcher->merge(0, sorter_x->clusterer->clusters);
  matcher->merge(1, sorter_y->clusterer->clusters);
  matcher->match_end(false);
  EXPECT_EQ(matcher->stats_cluster_count, 11);

  sorter_x->flush();
  sorter_y->flush();
  EXPECT_EQ(sorter_x->clusterer->stats_cluster_count, 20);
  EXPECT_EQ(sorter_y->clusterer->stats_cluster_count, 20);
  matcher->merge(0, sorter_x->clusterer->clusters);
  matcher->merge(1, sorter_y->clusterer->clusters);
  matcher->match_end(true);
  EXPECT_EQ(matcher->stats_cluster_count, 14);
}

TEST_F(ClusterMatcherTest, a10) {
  ReadoutFile::read(DataPath + "/readouts/a00010", readouts);
  EXPECT_EQ(readouts.size(), 920);

  uint64_t bonus = 0;
  uint64_t old = 0;
  for (auto readout : readouts) {
    if (readout.srs_timestamp < old)
      bonus++;
    old = readout.srs_timestamp;
    /// \todo this hack should not be necessary!
    readout.srs_timestamp += (bonus << 42);
    store_hit(readout);
  }

  /// \todo I don't trust these results anymore, please validate
  EXPECT_EQ(sorter_x->clusterer->stats_cluster_count, 94);
  EXPECT_EQ(sorter_y->clusterer->stats_cluster_count, 66);
  matcher->merge(0, sorter_x->clusterer->clusters);
  matcher->merge(1, sorter_y->clusterer->clusters);
  matcher->match_end(false);
  EXPECT_EQ(matcher->stats_cluster_count, 116);

  sorter_x->flush();
  sorter_y->flush();
  EXPECT_EQ(sorter_x->clusterer->stats_cluster_count, 96);
  EXPECT_EQ(sorter_y->clusterer->stats_cluster_count, 68);
  matcher->merge(0, sorter_x->clusterer->clusters);
  matcher->merge(1, sorter_y->clusterer->clusters);
  matcher->match_end(true);
  EXPECT_EQ(matcher->stats_cluster_count, 128);
}

TEST_F(ClusterMatcherTest, a100) {
  ReadoutFile::read(DataPath + "/readouts/a00100", readouts);
  EXPECT_EQ(readouts.size(), 126590);

  uint64_t bonus = 0;
  uint64_t old = 0;
  for (auto readout : readouts) {
    if (readout.srs_timestamp < old)
      bonus++;
    old = readout.srs_timestamp;
    /// \todo this hack should not be necessary!
    readout.srs_timestamp += (bonus << 42);
    store_hit(readout);
  }

  /// \todo I don't trust these results anymore, please validate
  EXPECT_EQ(sorter_x->clusterer->stats_cluster_count, 18986);
  EXPECT_EQ(sorter_y->clusterer->stats_cluster_count, 9727);
  matcher->merge(0, sorter_x->clusterer->clusters);
  matcher->merge(1, sorter_y->clusterer->clusters);
  matcher->match_end(false);
  EXPECT_EQ(matcher->stats_cluster_count, 24806);

  sorter_x->flush();
  sorter_y->flush();
  EXPECT_EQ(sorter_x->clusterer->stats_cluster_count, 18991);
  EXPECT_EQ(sorter_y->clusterer->stats_cluster_count, 9731);
  matcher->merge(0, sorter_x->clusterer->clusters);
  matcher->merge(1, sorter_y->clusterer->clusters);
  matcher->match_end(true);
  EXPECT_EQ(matcher->stats_cluster_count, 25094);
}


// \todo test how many have both planes

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
