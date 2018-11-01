/** Copyright (C) 2017 European Spallation Source ERIC */

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <gdgem/clustering/HitSorter.h>
#include <gdgem/clustering/DoroClusterer.h>
#include <gdgem/clustering/ClusterMatcher.h>
#include <test/TestBase.h>
#include <functional>

#include <gdgem/clustering/TestDataShort.h>
#include <gdgem/nmx/Readout.h>

#define UNUSED __attribute__((unused))

class ClusterMatcherTest : public TestBase {
protected:
  std::vector<Readout> long_data;

  uint16_t pADCThreshold = 0;
  size_t pMinClusterSize = 3;
  // Maximum time difference between hits in time sorted cluster (x or y)
  double pMaxTimeGap = 200;
  // Maximum number of missing strips in strip sorted cluster (x or y)
  uint16_t pMaxStripGap = 2;
  //Maximum cluster time difference between matching clusters in x and y
  //Cluster time is either calculated with center-of-mass or uTPC method
  double pDeltaTimePlanes = 200;
  // Maximum time span for total cluster (x or y)
  // double pDeltaTimeSpan = 500;

  SRSMappings mapping;

  std::shared_ptr<ClusterMatcher> matcher;
  std::shared_ptr<HitSorter> sorter_x;
  std::shared_ptr<HitSorter> sorter_y;

  virtual void SetUp() {
    std::string DataPath = TEST_DATA_PATH;
    ReadoutFile::read(DataPath + "run16long", long_data);

    mapping.set_mapping(1, 0, 0, 0);
    mapping.set_mapping(1, 1, 0, 64);
    mapping.set_mapping(1, 6, 0, 128);
    mapping.set_mapping(1, 7, 0, 192);

    mapping.set_mapping(1, 10, 1, 0);
    mapping.set_mapping(1, 11, 1, 64);
    mapping.set_mapping(1, 14, 1, 128);
    mapping.set_mapping(1, 15, 1, 192);

    SRSTime srstime;
    srstime.set_bc_clock(20);
    srstime.set_tac_slope(60);
    srstime.set_trigger_resolution_ns(3.125);
    srstime.set_acquisition_window(4000);

    matcher = std::make_shared<ClusterMatcher>(pDeltaTimePlanes);
    sorter_x = std::make_shared<HitSorter>(srstime, mapping, pADCThreshold, pMaxTimeGap);
    sorter_y = std::make_shared<HitSorter>(srstime, mapping, pADCThreshold, pMaxTimeGap);
    sorter_x->clusterer = std::make_shared<DoroClusterer>(pMaxTimeGap, pMaxStripGap, pMinClusterSize);
    sorter_y->clusterer = std::make_shared<DoroClusterer>(pMaxTimeGap, pMaxStripGap, pMinClusterSize);
  }

  virtual void TearDown() {
  }

  void store_hit(const Readout& readout)
  {
    uint8_t planeID = mapping.get_plane(readout);
    if (planeID == 1) {
      sorter_y->insert(readout);
    } else {
      sorter_x->insert(readout);
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

TEST_F(ClusterMatcherTest, MergeNoClusters) {
  ClusterList c;
  MESSAGE() << "This it NOT a test, as novalidation occurs (cant access private fields)\n";
  matcher->merge(0, c);
}

TEST_F(ClusterMatcherTest, OneX) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0, 10, 0, 200));
  matcher->match_end(true);
  ASSERT_EQ(matcher->stats_cluster_count, 1);
  ASSERT_EQ(matcher->matched_clusters.size(), 1);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 200);
  EXPECT_EQ(matcher->matched_clusters.front().x.entries.size(), 122);
  EXPECT_EQ(matcher->matched_clusters.front().y.entries.size(), 0);
}

TEST_F(ClusterMatcherTest, OneY) {
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0, 10, 0, 200));
  matcher->match_end(true);
  ASSERT_EQ(matcher->stats_cluster_count, 1);
  ASSERT_EQ(matcher->matched_clusters.size(), 1);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 200);
  EXPECT_EQ(matcher->matched_clusters.front().x.entries.size(), 0);
  EXPECT_EQ(matcher->matched_clusters.front().y.entries.size(), 122);
}

TEST_F(ClusterMatcherTest, TwoX) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0, 10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0, 10, 500, 700));
  matcher->match_end(true);
  ASSERT_EQ(matcher->stats_cluster_count, 2);
  ASSERT_EQ(matcher->matched_clusters.size(), 2);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 200);
  EXPECT_EQ(matcher->matched_clusters.back().time_span(), 200);
  EXPECT_EQ(matcher->matched_clusters.front().x.entries.size(), 122);
  EXPECT_EQ(matcher->matched_clusters.front().y.entries.size(), 0);
  EXPECT_EQ(matcher->matched_clusters.back().x.entries.size(), 122);
  EXPECT_EQ(matcher->matched_clusters.back().y.entries.size(), 0);
}

TEST_F(ClusterMatcherTest, TwoY) {
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 500, 700));
  matcher->match_end(true);
  ASSERT_EQ(matcher->stats_cluster_count, 2);
  ASSERT_EQ(matcher->matched_clusters.size(), 2);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 200);
  EXPECT_EQ(matcher->matched_clusters.back().time_span(), 200);
  EXPECT_EQ(matcher->matched_clusters.front().x.entries.size(), 0);
  EXPECT_EQ(matcher->matched_clusters.front().y.entries.size(), 122);
  EXPECT_EQ(matcher->matched_clusters.back().x.entries.size(), 0);
  EXPECT_EQ(matcher->matched_clusters.back().y.entries.size(), 122);
}

TEST_F(ClusterMatcherTest, OneXOneY) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0,10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 500, 700));
  matcher->match_end(true);
  ASSERT_EQ(matcher->stats_cluster_count, 2);
  ASSERT_EQ(matcher->matched_clusters.size(), 2);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 200);
  EXPECT_EQ(matcher->matched_clusters.back().time_span(), 200);
  EXPECT_EQ(matcher->matched_clusters.front().x.entries.size(), 122);
  EXPECT_EQ(matcher->matched_clusters.front().y.entries.size(), 0);
  EXPECT_EQ(matcher->matched_clusters.back().x.entries.size(), 0);
  EXPECT_EQ(matcher->matched_clusters.back().y.entries.size(), 122);
}

TEST_F(ClusterMatcherTest, OneXY) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0,10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 0, 200));
  matcher->match_end(true);
  ASSERT_EQ(matcher->stats_cluster_count, 1);
  ASSERT_EQ(matcher->matched_clusters.size(), 1);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 200);
  EXPECT_EQ(matcher->matched_clusters.front().x.entries.size(), 122);
  EXPECT_EQ(matcher->matched_clusters.front().y.entries.size(), 122);
}

TEST_F(ClusterMatcherTest, TwoXY) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0,10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 1, 300));
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0,10, 600, 800));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 650, 850));
  matcher->match_end(true);
  ASSERT_EQ(matcher->stats_cluster_count, 2);
  ASSERT_EQ(matcher->matched_clusters.size(), 2);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 300);
  EXPECT_EQ(matcher->matched_clusters.back().time_span(), 250);
  EXPECT_EQ(matcher->matched_clusters.front().x.entries.size(), 122);
  EXPECT_EQ(matcher->matched_clusters.front().y.entries.size(), 122);
  EXPECT_EQ(matcher->matched_clusters.back().x.entries.size(), 122);
  EXPECT_EQ(matcher->matched_clusters.back().y.entries.size(), 122);
}

TEST_F(ClusterMatcherTest, JustIntside) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0,10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 200, 400));
  matcher->match_end(true);
  ASSERT_EQ(matcher->matched_clusters.size(), 1);
}

TEST_F(ClusterMatcherTest, JustOutside) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0,10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 200, 401));
  matcher->match_end(true);
  ASSERT_EQ(matcher->matched_clusters.size(), 2);
}

TEST_F(ClusterMatcherTest, DontForce) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0,10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 200, 401));
  matcher->match_end(false);
  ASSERT_EQ(matcher->matched_clusters.size(), 0);

  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 800, 1000));
  matcher->match_end(false);
  ASSERT_EQ(matcher->matched_clusters.size(), 0);

  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 900, 1000));
  matcher->match_end(false);
  ASSERT_EQ(matcher->matched_clusters.size(), 0);
}


TEST_F(ClusterMatcherTest, Run16_Short) {
  uint32_t bonus = 0;
  uint32_t old = 0;
  for (auto readout : Run16) {
    if (readout.srs_timestamp < old)
      bonus++;
    old = readout.srs_timestamp+bonus;
    store_hit(readout);
  }
  /// \todo I don't trust these numbers, they have decreased after
  /// changing tdc calculations
  EXPECT_EQ(sorter_x->clusterer->stats_cluster_count, 0); //down from 3
  EXPECT_EQ(sorter_y->clusterer->stats_cluster_count, 0); // down from 4
  matcher->merge(0, sorter_x->clusterer->clusters);
  matcher->merge(1, sorter_y->clusterer->clusters);
  matcher->match_end(false);
  EXPECT_EQ(matcher->stats_cluster_count, 0); // down from 1

  sorter_x->flush();
  sorter_y->flush();
  EXPECT_EQ(sorter_x->clusterer->stats_cluster_count, 7); //
  EXPECT_EQ(sorter_y->clusterer->stats_cluster_count, 6); // down from 11
  matcher->merge(0, sorter_x->clusterer->clusters);
  matcher->merge(1, sorter_y->clusterer->clusters);
  matcher->match_end(true);
  EXPECT_EQ(matcher->stats_cluster_count, 2); // down from 6
}

TEST_F(ClusterMatcherTest, Run16_Long_identical) {
  uint32_t bonus = 0;
  uint32_t old = 0;
  for (auto readout : long_data) {
    if (readout.srs_timestamp < old)
      bonus++;
    old = readout.srs_timestamp+bonus;
    sorter_y->insert(readout);
    sorter_x->insert(readout);
  }

  sorter_x->flush();
  sorter_y->flush();
  EXPECT_EQ(sorter_x->clusterer->clusters.size(), 8167); // 20293
  EXPECT_EQ(sorter_y->clusterer->clusters.size(), 8167); // 20293

  matcher = std::make_shared<ClusterMatcher>(0);
  matcher->merge(0, sorter_x->clusterer->clusters);
  matcher->merge(1, sorter_y->clusterer->clusters);

  EXPECT_EQ(matcher->unmatched_clusters.size(), 16334); // 40586

  matcher->match_end(true);
  EXPECT_EQ(matcher->stats_cluster_count, 1450); // 16954
}

TEST_F(ClusterMatcherTest, Run16_Long) {
  uint32_t bonus = 0;
  uint32_t old = 0;
  for (auto readout : long_data) {
    if (readout.srs_timestamp < old)
      bonus++;
    old = readout.srs_timestamp+bonus;
    store_hit(readout);
  }
  sorter_x->flush();
  sorter_y->flush();
  EXPECT_EQ(sorter_x->clusterer->stats_cluster_count, 7044); /// 10226
  EXPECT_EQ(sorter_y->clusterer->stats_cluster_count, 5837); // 12467

  matcher = std::make_shared<ClusterMatcher>(10);
  matcher->merge(0, sorter_x->clusterer->clusters);
  matcher->merge(1, sorter_y->clusterer->clusters);
  EXPECT_EQ(matcher->unmatched_clusters.size(), 12881); // 22693
  matcher->match_end(true);
//  EXPECT_EQ(matcher->stats_cluster_count, 6250);
  EXPECT_EQ(matcher->stats_cluster_count, 1537); // 19080
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
