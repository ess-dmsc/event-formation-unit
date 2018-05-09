/** Copyright (C) 2017 European Spallation Source ERIC */

#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <gdgem/vmm2srs/SRSMappings.h>
#include <gdgem/vmm2srs/SRSTime.h>
#include <gdgem/dg_impl/NMXClusterer.h>
#include <gdgem/dg_impl/TestData.h>
#include <gdgem/dg_impl/TestDataLong.h>
#include <test/TestBase.h>
#include <functional>

#define UNUSED __attribute__((unused))

class NMXClustererTest : public TestBase {
protected:
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
    clusters_x = std::make_shared<NMXClusterer>(srstime, pMinClusterSize, pDeltaTimeHits, pDeltaStripHits, pDeltaTimeSpan);
    clusters_y = std::make_shared<NMXClusterer>(srstime, pMinClusterSize, pDeltaTimeHits, pDeltaStripHits, pDeltaTimeSpan);
    sorter_x = std::make_shared<NMXHitSorter>(srstime, mapping, pADCThreshold,  pDeltaTimeHits, *clusters_x);
    sorter_y = std::make_shared<NMXHitSorter>(srstime, mapping, pADCThreshold,  pDeltaTimeHits, *clusters_y);
  }

  virtual void TearDown() {
  }

  uint16_t pADCThreshold = 0;
  size_t pMinClusterSize = 3;
  //Maximum time difference between strips in time sorted cluster (x or y)
  float pDeltaTimeHits = 200;
  //Number of missing strips in strip sorted cluster (x or y)
  uint16_t pDeltaStripHits = 2;
  //Maximum time span for total cluster (x or y)
  float pDeltaTimeSpan = 500;
  //Maximum cluster time difference between matching clusters in x and y
  //Cluster time is either calculated with center-of-mass or uTPC method
  float pDeltaTimePlanes = 200;

  SRSMappings mapping;

  std::shared_ptr<NMXClusterMatcher> matcher;
  std::shared_ptr<NMXClusterer> clusters_x;
  std::shared_ptr<NMXClusterer> clusters_y;
  std::shared_ptr<NMXHitSorter> sorter_x;
  std::shared_ptr<NMXHitSorter> sorter_y;
};

TEST_F(NMXClustererTest, Run16_line_110168_110323) {
  for (auto hit : Run16) { // replace with UDP receive()
    uint8_t planeID = mapping.get_plane(hit.fec, hit.chip_id);
    if (planeID == 1) {
      sorter_y->AnalyzeHits(hit.srs_timestamp, hit.framecounter,
                            hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                            hit.adc,
                            hit.overthreshold);
    } else {
      sorter_x->AnalyzeHits(hit.srs_timestamp, hit.framecounter,
                            hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                            hit.adc,
                            hit.overthreshold);
    }

//    if (clusters_x->ready() && clusters_y->ready())
//    {
//      matcher->match(clusters_x->clusters, clusters_y->clusters);
//    }
  }
  EXPECT_EQ(0, sorter_x->stats_triggertime_wraps);
  EXPECT_EQ(0, sorter_x->stats_fc_error);
  EXPECT_EQ(0, sorter_x->stats_bcid_tdc_error);
  EXPECT_EQ(0, sorter_x->stats_fc_error);
  EXPECT_EQ(0, sorter_y->stats_triggertime_wraps);
  EXPECT_EQ(0, sorter_y->stats_fc_error);
  EXPECT_EQ(0, sorter_y->stats_bcid_tdc_error);
  EXPECT_EQ(0, sorter_y->stats_fc_error);
  EXPECT_EQ(clusters_x->stats_cluster_count, 5);
  EXPECT_EQ(clusters_y->stats_cluster_count, 9);

  matcher->match_end(clusters_x->clusters, clusters_y->clusters);
  EXPECT_EQ(matcher->stats_cluster_count, 4);
}


TEST_F(NMXClustererTest, Run16_Long) {
  for (auto hit : Run16_Long) { // replace with UDP receive()
    uint8_t planeID = mapping.get_plane(hit.fec, hit.chip_id);
    if (planeID == 1) {
      sorter_y->AnalyzeHits(hit.srs_timestamp, hit.framecounter,
                            hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                            hit.adc,
                            hit.overthreshold);
    } else {
      sorter_x->AnalyzeHits(hit.srs_timestamp, hit.framecounter,
                            hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                            hit.adc,
                            hit.overthreshold);
    }

//    if (clusters_x->ready() && clusters_y->ready())
//    {
//      matcher->match(clusters_x->clusters, clusters_y->clusters);
//    }
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

  matcher->match_end(clusters_x->clusters, clusters_y->clusters);
  EXPECT_EQ(matcher->stats_cluster_count, 10111);
}

TEST_F(NMXClustererTest, BcidTdcError) {
  EXPECT_EQ(0, sorter_x->stats_bcid_tdc_error);
  EXPECT_EQ(0, sorter_y->stats_bcid_tdc_error);

  for (auto hit : err_bcid_tdc_error) {
    uint8_t planeID = mapping.get_plane(hit.fec, hit.chip_id);
    if (planeID == 1) {
      sorter_y->AnalyzeHits(hit.srs_timestamp, hit.framecounter,
                            hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                            hit.adc,
                            hit.overthreshold);
    } else {
      sorter_x->AnalyzeHits(hit.srs_timestamp, hit.framecounter,
                            hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                            hit.adc,
                            hit.overthreshold);
    }
  }
  // Two in X and Two in Y
  EXPECT_EQ(2, sorter_x->stats_bcid_tdc_error);
  EXPECT_EQ(2, sorter_y->stats_bcid_tdc_error);
}

#if 0

TEST_F(NMXClustererTest, FrameCounterError) {
  EXPECT_EQ(0, sorter_x->stats_fc_error);
  EXPECT_EQ(0, sorter_y->stats_fc_error);

  for (auto hit : err_fc_error) {
    uint8_t planeID = mapping.get_plane(hit.fec, hit.chip_id);
    if (planeID == 1) {
      sorter_y->AnalyzeHits(hit.srs_timestamp, hit.framecounter,
                            hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                            hit.adc,
                            hit.overthreshold);
    } else {
      sorter_x->AnalyzeHits(hit.srs_timestamp, hit.framecounter,
                            hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                            hit.adc,
                            hit.overthreshold);
    }
  }
  EXPECT_EQ(1, sorter_x->stats_fc_error);
  EXPECT_EQ(1, sorter_y->stats_fc_error);
}

TEST_F(NMXClustererTest, TriggerTimeWraps) {
  EXPECT_EQ(0, sorter_x->stats_triggertime_wraps);
  EXPECT_EQ(0, sorter_y->stats_triggertime_wraps);

  for (auto hit : err_triggertime_error) {
    uint8_t planeID = mapping.get_plane(hit.fec, hit.chip_id);
    if (planeID == 1) {
      sorter_y->AnalyzeHits(hit.srs_timestamp, hit.framecounter,
                            hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                            hit.adc,
                            hit.overthreshold);
    } else {
      sorter_x->AnalyzeHits(hit.srs_timestamp, hit.framecounter,
                            hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                            hit.adc,
                            hit.overthreshold);
    }
  }
  EXPECT_EQ(1, sorter_x->stats_triggertime_wraps);
  EXPECT_EQ(1, sorter_y->stats_triggertime_wraps);
}

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
    NMXClusterer nmxdata1(pBC, pTAC, pAcqWin, pXChips, pYChips, pADCThreshold, pMinClusterSize, pDeltaTimeHits, pDeltaStripHits,pDeltaTimeSpan,pDeltaTimePlanes);
    NMXClusterer nmxdata2(pBC, pTAC, pAcqWin, pXChips, pYChips, pADCThreshold, pMinClusterSize, pDeltaTimeHits, pDeltaStripHits,pDeltaTimeSpan,pDeltaTimePlanes);

    for (auto hit : Run16_line_110168_110323) { // replace with UDP receive()
        printf("fec: %d, asic: %d, ch: %d -> %d\n", hit.fec, hit.chip_id, hit.channel, distributeto(hit.fec, hit.chip_id, hit.channel, 10));
        if (distributeto(hit.fec, hit.chip_id, hit.channel, 10) & 0x01) {
            nmxdata1.AnalyzeHits(hit.srs_timestamp, hit.framecounter, hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc, hit.adc, hit.overthreshold);
        }
        if (distributeto(hit.fec, hit.chip_id, hit.channel, 10) & 0x02) {
            nmxdata2.AnalyzeHits(hit.srs_timestamp, hit.framecounter, hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc, hit.adc, hit.overthreshold);
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
