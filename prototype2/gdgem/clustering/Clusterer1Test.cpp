/** Copyright (C) 2017 European Spallation Source ERIC */

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <gdgem/clustering/HitSorter.h>
#include <gdgem/clustering/Clusterer1.h>
#include <test/TestBase.h>
#include <functional>

#include <gdgem/clustering/TestDataShort.h>

#define UNUSED __attribute__((unused))

class NMXClustererTest : public TestBase {
protected:
  SRSHitIO long_data;

  uint16_t pADCThreshold = 0;
  size_t pMinClusterSize = 3;
  // Maximum time difference between hits in time sorted cluster (x or y)
  double pMaxTimeGap = 200;
  // Maximum number of missing strips in strip sorted cluster (x or y)
  uint16_t pMaxStripGap = 2;

  SRSMappings mapping;

  std::shared_ptr<AbstractClusterer> clusters_x;
  std::shared_ptr<AbstractClusterer> clusters_y;
  std::shared_ptr<HitSorter> sorter_x;
  std::shared_ptr<HitSorter> sorter_y;

  virtual void SetUp() {
    std::string DataPath = TEST_DATA_PATH;
    long_data.read(DataPath + "Run16Long.h5");

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

    clusters_x = std::make_shared<Clusterer1>(pMaxTimeGap, pMaxStripGap, pMinClusterSize);
    clusters_y = std::make_shared<Clusterer1>(pMaxTimeGap, pMaxStripGap, pMinClusterSize);
    sorter_x = std::make_shared<HitSorter>(srstime, mapping, pADCThreshold, pMaxTimeGap, clusters_x);
    sorter_y = std::make_shared<HitSorter>(srstime, mapping, pADCThreshold, pMaxTimeGap, clusters_y);
  }

  virtual void TearDown() {
  }
};

// Test this without sorter!!!
// Use presorted data that we understand

TEST_F(NMXClustererTest, Run16_line_110168_110323) {
  for (const auto& hit : Run16) {
    uint8_t planeID = mapping.get_plane(hit.fec, hit.chip_id);
    if (planeID == 1) {
      sorter_y->store(hit.srs_timestamp, hit.frame_counter,
                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                      hit.adc,
                      hit.over_threshold);
    } else {
      sorter_x->store(hit.srs_timestamp, hit.frame_counter,
                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                      hit.adc,
                      hit.over_threshold);
    }
  }
  EXPECT_EQ(clusters_x->stats_cluster_count, 3);
  EXPECT_EQ(clusters_y->stats_cluster_count, 4);
}

TEST_F(NMXClustererTest, Run16_Long) {
  for (const auto& hit : long_data.data) {
    uint8_t planeID = mapping.get_plane(hit.fec, hit.chip_id);
    if (planeID == 1) {
      sorter_y->store(hit.srs_timestamp, hit.frame_counter,
                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                      hit.adc,
                      hit.over_threshold);
    } else {
      sorter_x->store(hit.srs_timestamp, hit.frame_counter,
                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                      hit.adc,
                      hit.over_threshold);
    }
  }
  EXPECT_EQ(clusters_x->stats_cluster_count, 10198);
  EXPECT_EQ(clusters_y->stats_cluster_count, 12432);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
