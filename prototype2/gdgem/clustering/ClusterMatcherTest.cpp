/** Copyright (C) 2017 European Spallation Source ERIC */

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <gdgem/clustering/HitSorter.h>
#include <gdgem/clustering/Clusterer1.h>
#include <gdgem/clustering/ClusterMatcher.h>
#include <test/TestBase.h>
#include <functional>

#include <gdgem/clustering/TestDataShort.h>
#include <gdgem/nmx/ReadoutFile.h>

#define UNUSED __attribute__((unused))

class NMXClustererTest : public TestBase {
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
  std::shared_ptr<AbstractClusterer> clusters_x;
  std::shared_ptr<AbstractClusterer> clusters_y;
  std::shared_ptr<HitSorter> sorter_x;
  std::shared_ptr<HitSorter> sorter_y;

  virtual void SetUp() {
    std::string DataPath = TEST_DATA_PATH;
    ReadoutFile::read(DataPath + "run16long.h5", long_data);

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
    srstime.set_trigger_resolution(3.125);
    srstime.set_acquisition_window(4000);

    matcher = std::make_shared<ClusterMatcher>(pDeltaTimePlanes);
    clusters_x = std::make_shared<Clusterer1>(pMaxTimeGap, pMaxStripGap, pMinClusterSize);
    clusters_y = std::make_shared<Clusterer1>(pMaxTimeGap, pMaxStripGap, pMinClusterSize);
    sorter_x = std::make_shared<HitSorter>(srstime, mapping, pADCThreshold, pMaxTimeGap, clusters_x);
    sorter_y = std::make_shared<HitSorter>(srstime, mapping, pADCThreshold, pMaxTimeGap, clusters_y);
  }

  virtual void TearDown() {
  }

  void store_hit(const Readout& readout)
  {
    uint8_t planeID = mapping.get_plane(readout.fec, readout.chip_id);
    if (planeID == 1) {
      sorter_y->store(readout.srs_timestamp, readout.frame_counter,
                      readout.fec, readout.chip_id, readout.channel, readout.bcid, readout.tdc,
                      readout.adc,
                      readout.over_threshold);
    } else {
      sorter_x->store(readout.srs_timestamp, readout.frame_counter,
                      readout.fec, readout.chip_id, readout.channel, readout.bcid, readout.tdc,
                      readout.adc,
                      readout.over_threshold);
    }
  }

  Cluster mock_cluster(uint8_t plane, uint16_t strip_start, uint16_t strip_end,
                       double time_start, double time_end) {
    Cluster ret;
    Eventlet e;
    e.plane_id = plane;
    e.adc = 1;
    double time_step = (time_end - time_start) / 10.0;
    for (e.time = time_start; e.time <= time_end; e.time += time_step)
      for (e.strip = strip_start; e.strip <= strip_end; ++e.strip)
        ret.insert_eventlet(e);
    e.time = time_end;
    ret.insert_eventlet(e);
    return ret;
  }

};

TEST_F(NMXClustererTest, OneX) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0, 10, 0, 200));
  matcher->match_end(true);
  ASSERT_EQ(matcher->stats_cluster_count, 1);
  ASSERT_EQ(matcher->matched_clusters.size(), 1);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 200);
  EXPECT_EQ(matcher->matched_clusters.front().x.entries.size(), 122);
  EXPECT_EQ(matcher->matched_clusters.front().y.entries.size(), 0);
}

TEST_F(NMXClustererTest, OneY) {
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0, 10, 0, 200));
  matcher->match_end(true);
  ASSERT_EQ(matcher->stats_cluster_count, 1);
  ASSERT_EQ(matcher->matched_clusters.size(), 1);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 200);
  EXPECT_EQ(matcher->matched_clusters.front().x.entries.size(), 0);
  EXPECT_EQ(matcher->matched_clusters.front().y.entries.size(), 122);
}

TEST_F(NMXClustererTest, TwoX) {
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

TEST_F(NMXClustererTest, TwoY) {
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

TEST_F(NMXClustererTest, OneXOneY) {
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

TEST_F(NMXClustererTest, OneXY) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0,10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 0, 200));
  matcher->match_end(true);
  ASSERT_EQ(matcher->stats_cluster_count, 1);
  ASSERT_EQ(matcher->matched_clusters.size(), 1);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 200);
  EXPECT_EQ(matcher->matched_clusters.front().x.entries.size(), 122);
  EXPECT_EQ(matcher->matched_clusters.front().y.entries.size(), 122);
}

TEST_F(NMXClustererTest, TwoXY) {
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

TEST_F(NMXClustererTest, JustIntside) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0,10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 200, 400));
  matcher->match_end(true);
  ASSERT_EQ(matcher->matched_clusters.size(), 1);
}

TEST_F(NMXClustererTest, JustOutside) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0,10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 200, 401));
  matcher->match_end(true);
  ASSERT_EQ(matcher->matched_clusters.size(), 2);
}

TEST_F(NMXClustererTest, DontForce) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0,10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 200, 401));
  matcher->match_end(false);
  ASSERT_EQ(matcher->matched_clusters.size(), 0);

  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 800, 1000));
  matcher->match_end(false);
  ASSERT_EQ(matcher->matched_clusters.size(), 0);

  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 900, 1000));
  matcher->match_end(false);
  ASSERT_EQ(matcher->matched_clusters.size(), 1);
}


TEST_F(NMXClustererTest, Run16_line_110168_110323) {
  for (const auto& readout : Run16) {
    store_hit(readout);
  }
  EXPECT_EQ(clusters_x->stats_cluster_count, 3);
  EXPECT_EQ(clusters_y->stats_cluster_count, 4);
  matcher->merge(clusters_x->clusters);
  matcher->merge(clusters_y->clusters);
  matcher->match_end(false);
  EXPECT_EQ(matcher->stats_cluster_count, 2);

  sorter_x->flush();
  sorter_y->flush();
  EXPECT_EQ(clusters_x->stats_cluster_count, 7);
  EXPECT_EQ(clusters_y->stats_cluster_count, 11);
  matcher->merge(clusters_x->clusters);
  matcher->merge(clusters_y->clusters);
  matcher->match_end(true);
  EXPECT_EQ(matcher->stats_cluster_count, 7);
}

TEST_F(NMXClustererTest, Run16_Long_identical) {
  for (const auto& readout : long_data) {
    sorter_y->store(readout.srs_timestamp, readout.frame_counter,
                    readout.fec, readout.chip_id, readout.channel, readout.bcid, readout.tdc,
                    readout.adc,
                    readout.over_threshold);
    sorter_x->store(readout.srs_timestamp, readout.frame_counter,
                    readout.fec, readout.chip_id, readout.channel, readout.bcid, readout.tdc,
                    readout.adc,
                    readout.over_threshold);
  }
  sorter_x->flush();
  sorter_y->flush();
  EXPECT_EQ(clusters_x->clusters.size(), 20278);
  EXPECT_EQ(clusters_y->clusters.size(), 20278);

  matcher = std::make_shared<ClusterMatcher>(0);
  matcher->merge(clusters_x->clusters);
  matcher->merge(clusters_y->clusters);

  EXPECT_EQ(matcher->unmatched_clusters.size(), 40556);

  matcher->match_end(true);
  EXPECT_EQ(matcher->stats_cluster_count, 17504);
  //may be separated in time but remerged here
}

TEST_F(NMXClustererTest, Run16_Long) {
  for (const auto& readout : long_data) {
    store_hit(readout);
  }
  sorter_x->flush();
  sorter_y->flush();
  EXPECT_EQ(clusters_x->stats_cluster_count, 10203);
  EXPECT_EQ(clusters_y->stats_cluster_count, 12444);

  matcher = std::make_shared<ClusterMatcher>(10);
  matcher->merge(clusters_x->clusters);
  matcher->merge(clusters_y->clusters);
  EXPECT_EQ(matcher->unmatched_clusters.size(), 22647);
  matcher->match_end(true);
  EXPECT_EQ(matcher->stats_cluster_count, 6141);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
