/** Copyright (C) 2017 European Spallation Source ERIC */

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <gdgem/dg_impl/HitSorter.h>
#include <gdgem/dg_impl/Clusterer1.h>
#include <gdgem/dg_impl/ClusterMatcher.h>
#include <test/TestBase.h>
#include <functional>

#include <gdgem/dg_impl/TestDataShort.h>
#include <gdgem/dg_impl/TestDataLong.h>


#define UNUSED __attribute__((unused))

class NMXClustererTest : public TestBase {
protected:
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
};

// Test these without clusterer, with preclustered data in both dimensions

TEST_F(NMXClustererTest, Run16_line_110168_110323) {
  for (auto hit : Run16) {
    uint8_t planeID = mapping.get_plane(hit.fec, hit.chip_id);
    if (planeID == 1) {
      sorter_y->store(hit.srs_timestamp, hit.framecounter,
                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                      hit.adc,
                      hit.overthreshold);
    } else {
      sorter_x->store(hit.srs_timestamp, hit.framecounter,
                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                      hit.adc,
                      hit.overthreshold);
    }
  }
  EXPECT_EQ(clusters_x->stats_cluster_count, 3);
  EXPECT_EQ(clusters_y->stats_cluster_count, 4);
  matcher->match_end(clusters_x->clusters, clusters_y->clusters, true);
  EXPECT_EQ(matcher->stats_cluster_count, 2);
}

TEST_F(NMXClustererTest, Run16_Long) {
  for (auto hit : Run16_Long) {
    uint8_t planeID = mapping.get_plane(hit.fec, hit.chip_id);
    if (planeID == 1) {
      sorter_y->store(hit.srs_timestamp, hit.framecounter,
                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                      hit.adc,
                      hit.overthreshold);
    } else {
      sorter_x->store(hit.srs_timestamp, hit.framecounter,
                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                      hit.adc,
                      hit.overthreshold);
    }
  }
  EXPECT_EQ(clusters_x->stats_cluster_count, 10198);
  EXPECT_EQ(clusters_y->stats_cluster_count, 12432);
  matcher->match_end(clusters_x->clusters, clusters_y->clusters, true);
  EXPECT_EQ(matcher->stats_cluster_count, 10111);
}

TEST_F(NMXClustererTest, Run16_Long_identical) {
  for (auto hit : Run16_Long) {
    uint8_t planeID = mapping.get_plane(hit.fec, hit.chip_id);
    if (planeID == 1) {
      sorter_y->store(hit.srs_timestamp, hit.framecounter,
                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                      hit.adc,
                      hit.overthreshold);
      sorter_x->store(hit.srs_timestamp, hit.framecounter,
                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                      hit.adc,
                      hit.overthreshold);
    }
  }
  EXPECT_EQ(clusters_x->stats_cluster_count, 12432);
  EXPECT_EQ(clusters_y->stats_cluster_count, 12432);
  matcher->match_end(clusters_x->clusters, clusters_y->clusters, true);
  EXPECT_EQ(matcher->stats_cluster_count, 12432);
}



int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
