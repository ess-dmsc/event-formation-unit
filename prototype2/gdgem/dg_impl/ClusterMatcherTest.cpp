/** Copyright (C) 2017 European Spallation Source ERIC */

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <gdgem/dg_impl/ClusterMatcher.h>
#include <gdgem/dg_impl/NMXSorter.h>
#include <test/TestBase.h>
#include <functional>

#include <gdgem/dg_impl/TestDataShort.h>
#include <gdgem/dg_impl/TestDataLong.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

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

  std::shared_ptr<NMXClusterMatcher> matcher;
  std::shared_ptr<NMXClusterer> clusters_x;
  std::shared_ptr<NMXClusterer> clusters_y;
  std::shared_ptr<NMXHitSorter> sorter_x;
  std::shared_ptr<NMXHitSorter> sorter_y;

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

    matcher = std::make_shared<NMXClusterMatcher>(pDeltaTimePlanes);
    clusters_x = std::make_shared<NMXClusterer>(pMaxTimeGap, pMaxStripGap, pMinClusterSize);
    clusters_y = std::make_shared<NMXClusterer>(pMaxTimeGap, pMaxStripGap, pMinClusterSize);
    sorter_x = std::make_shared<NMXHitSorter>(srstime, mapping, pADCThreshold, pMaxTimeGap, *clusters_x);
    sorter_y = std::make_shared<NMXHitSorter>(srstime, mapping, pADCThreshold, pMaxTimeGap, *clusters_y);
  }

  virtual void TearDown() {
  }
};

TEST_F(NMXClustererTest, Run16_line_110168_110323) {
  for (auto hit : Run16) { // replace with UDP receive()
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

    if (clusters_x->ready() && clusters_y->ready())
    {
      matcher->match_end(*clusters_x, *clusters_y, false);
    }
  }
  EXPECT_EQ(0, sorter_x->stats_triggertime_wraps);
  EXPECT_EQ(0, sorter_x->stats_fc_error);
  EXPECT_EQ(0, sorter_x->stats_bcid_tdc_error);
  EXPECT_EQ(0, sorter_x->stats_fc_error);
  EXPECT_EQ(0, sorter_y->stats_triggertime_wraps);
  EXPECT_EQ(0, sorter_y->stats_fc_error);
  EXPECT_EQ(0, sorter_y->stats_bcid_tdc_error);
  EXPECT_EQ(0, sorter_y->stats_fc_error);
  EXPECT_EQ(clusters_x->stats_cluster_count, 3);
  EXPECT_EQ(clusters_y->stats_cluster_count, 4);

   matcher->match_end(*clusters_x, *clusters_y, true);
   EXPECT_EQ(matcher->stats_cluster_count, 2);
}

TEST_F(NMXClustererTest, Run16_Long) {
  for (auto hit : Run16_Long) { // replace with UDP receive()
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

    if (clusters_x->ready() && clusters_y->ready())
    {
      DTRACE(DEB, "\tready\n");
      matcher->match_end(*clusters_x, *clusters_y, false);
    }
  }
  EXPECT_EQ(0, sorter_x->stats_triggertime_wraps);
  EXPECT_EQ(0, sorter_x->stats_fc_error);
  EXPECT_EQ(0, sorter_x->stats_bcid_tdc_error);
  EXPECT_EQ(0, sorter_x->stats_fc_error);
  EXPECT_EQ(0, sorter_y->stats_triggertime_wraps);
  EXPECT_EQ(0, sorter_y->stats_fc_error);
  EXPECT_EQ(0, sorter_y->stats_bcid_tdc_error);
  EXPECT_EQ(0, sorter_y->stats_fc_error);
  EXPECT_EQ(clusters_x->stats_cluster_count, 10198);
  EXPECT_EQ(clusters_y->stats_cluster_count, 12432);

  matcher->match_end(*clusters_x, *clusters_y, true);
  EXPECT_EQ(matcher->stats_cluster_count, 10111);
}

TEST_F(NMXClustererTest, BcidTdcError) {
  EXPECT_EQ(0, sorter_x->stats_bcid_tdc_error);
  EXPECT_EQ(0, sorter_y->stats_bcid_tdc_error);

  for (auto hit : err_bcid_tdc_error) {
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
  // Two in X and Two in Y
  EXPECT_EQ(2, sorter_x->stats_bcid_tdc_error);
  EXPECT_EQ(2, sorter_y->stats_bcid_tdc_error);
}

TEST_F(NMXClustererTest, FrameCounterError) {
  EXPECT_EQ(0, sorter_x->stats_fc_error);
  EXPECT_EQ(0, sorter_y->stats_fc_error);

  for (auto hit : err_fc_error) {
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
  EXPECT_EQ(1, sorter_x->stats_fc_error);
  EXPECT_EQ(0, sorter_y->stats_fc_error);
}

TEST_F(NMXClustererTest, TriggerTimeWraps) {
  EXPECT_EQ(0, sorter_x->stats_triggertime_wraps);
  EXPECT_EQ(0, sorter_y->stats_triggertime_wraps);

  for (auto hit : err_triggertime_error) {
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
  EXPECT_EQ(0, sorter_x->stats_triggertime_wraps);
  EXPECT_EQ(1, sorter_y->stats_triggertime_wraps);
}

#if 0

int distributeto(UNUSED int fec, int asic, int channel, int width) {
    int cutoff = 63 - width;
    if ((asic == 10) || (asic == 11 && (channel < cutoff)) )
    return 1;
    if ((asic == 15) || (asic == 14 && (channel >= width)) )
    return 2;
    return 3;
}

TEST_F(NMXClustererTest, TestLoadDistribution)
{
    NMXClusterer nmxdata1(pBC, pTAC, pAcqWin, pXChips, pYChips, pADCThreshold, pMinClusterSize, pMaxTimeGap, pMaxStripGap,pDeltaTimeSpan,pDeltaTimePlanes);
    NMXClusterer nmxdata2(pBC, pTAC, pAcqWin, pXChips, pYChips, pADCThreshold, pMinClusterSize, pMaxTimeGap, pMaxStripGap,pDeltaTimeSpan,pDeltaTimePlanes);

    for (auto hit : Run16_line_110168_110323) { // replace with UDP receive()
        printf("fec: %d, asic: %d, ch: %d -> %d\n", hit.fec, hit.chip_id, hit.channel, distributeto(hit.fec, hit.chip_id, hit.channel, 10));
        if (distributeto(hit.fec, hit.chip_id, hit.channel, 10) & 0x01) {
            nmxdata1.store(hit.srs_timestamp, hit.framecounter, hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc, hit.adc, hit.overthreshold);
        }
        if (distributeto(hit.fec, hit.chip_id, hit.channel, 10) & 0x02) {
            nmxdata2.store(hit.srs_timestamp, hit.framecounter, hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc, hit.adc, hit.overthreshold);
        }
    }
    EXPECT_EQ(nmxdata1.getNumClustersX() + nmxdata2.getNumClustersX(), 3);
    EXPECT_EQ(nmxdata1.getNumClustersY() + nmxdata2.getNumClustersY(), 4);
    EXPECT_EQ(nmxdata1.getNumClustersXY() + nmxdata2.getNumClustersXY(), 2);
    EXPECT_EQ(nmxdata1.getNumClustersXY_uTPC() + nmxdata1.getNumClustersXY_uTPC(), 2);
}
#endif

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
