/** Copyright (C) 2017 European Spallation Source ERIC */

#include <gdgem/nmx/ReadoutFile.h>
#include <gdgem/clustering/HitSorter.h>
#include <gdgem/clustering/DoroClusterer.h>

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <test/TestBase.h>
#include <functional>

#include <gdgem/clustering/TestDataShort.h>

#define UNUSED __attribute__((unused))

class DoroClustererTest : public TestBase {
protected:
  std::vector<Readout> long_data;

  uint16_t pADCThreshold = 0;
  size_t pMinClusterSize = 3;
  // Maximum time difference between hits in time sorted cluster (x or y)
  double pMaxTimeGap = 200;
  // Maximum number of missing strips in strip sorted cluster (x or y)
  uint16_t pMaxStripGap = 2;

  SRSMappings mapping;

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
    srstime.set_trigger_resolution_ns(3.125);
    srstime.set_acquisition_window(4000);

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
};

/// \todo Test this without sorter!!! Use presorted data that we understand

TEST_F(DoroClustererTest, Run16_line_110168_110323) {
  uint32_t bonus = 0;
  uint32_t old = 0;
  for (auto readout : Run16) {
    if (readout.srs_timestamp < old)
      bonus++;
    old = readout.srs_timestamp;
    readout.bonus_timestamp = bonus;
    store_hit(readout);
  }

  /// \todo I don't trust these results anymore, please validate
  EXPECT_EQ(sorter_x->clusterer->stats_cluster_count, 0);
  EXPECT_EQ(sorter_y->clusterer->stats_cluster_count, 0);
  sorter_x->flush();
  sorter_y->flush();
  EXPECT_EQ(sorter_x->clusterer->stats_cluster_count, 7);
  EXPECT_EQ(sorter_y->clusterer->stats_cluster_count, 6);
}

TEST_F(DoroClustererTest, Run16_Long) {
  uint32_t bonus = 0;
  uint32_t old = 0;
  for (auto readout : long_data) {
    if (readout.srs_timestamp < old)
      bonus++;
    old = readout.srs_timestamp;
    readout.bonus_timestamp = bonus;
    store_hit(readout);
  }
  /// \todo counts have gone down after changing tdc calculations
  EXPECT_EQ(sorter_x->clusterer->stats_cluster_count, 7041); // down from 10221, down from ...
  EXPECT_EQ(sorter_y->clusterer->stats_cluster_count, 5830); // down from 12455, down from ...
  sorter_x->flush();
  sorter_y->flush();
  EXPECT_EQ(sorter_x->clusterer->stats_cluster_count, 7044); // down from 10226, ...
  EXPECT_EQ(sorter_y->clusterer->stats_cluster_count, 5837); // down from 12467, ...
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
