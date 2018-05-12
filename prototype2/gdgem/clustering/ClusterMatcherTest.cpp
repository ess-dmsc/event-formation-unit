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
    ReadoutFile::read(DataPath + "Run16Long.h5", long_data);

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
};

// Test these without clusterer, with preclustered data in both dimensions

TEST_F(NMXClustererTest, Run16_line_110168_110323) {
  for (const auto& readout : Run16) {
    store_hit(readout);
  }
  sorter_x->flush();
  sorter_y->flush();
  EXPECT_EQ(clusters_x->stats_cluster_count, 7);
  EXPECT_EQ(clusters_y->stats_cluster_count, 11);
  matcher->match_end(clusters_x->clusters, clusters_y->clusters, true);
//  EXPECT_EQ(matcher->stats_cluster_count, 2);
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
  EXPECT_EQ(clusters_x->stats_cluster_count, 20278);
  EXPECT_EQ(clusters_y->stats_cluster_count, 20278);
  matcher->match_end(clusters_x->clusters, clusters_y->clusters, true);
//  EXPECT_EQ(matcher->stats_cluster_count, 20278);
}

TEST_F(NMXClustererTest, Run16_Long) {
  for (const auto& readout : long_data) {
    store_hit(readout);
  }
  sorter_x->flush();
  sorter_y->flush();
  EXPECT_EQ(clusters_x->stats_cluster_count, 10203);
  EXPECT_EQ(clusters_y->stats_cluster_count, 12444);
//  matcher->match_end(clusters_x->clusters, clusters_y->clusters, true);
//  EXPECT_EQ(matcher->stats_cluster_count, 10111);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
