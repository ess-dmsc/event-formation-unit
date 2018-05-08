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

#define UNUSED __attribute__((unused))

class NMXClustererTest : public TestBase {
protected:
  virtual void SetUp() {
    SRSMappings mapping;

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

    nmxdata =
        new NMXClusterer(srstime, mapping, pADCThreshold, pMinClusterSize, pDeltaTimeHits, pDeltaStripHits,
                         pDeltaTimeSpan, pDeltaTimePlanes);
  }

  virtual void TearDown() {
    delete nmxdata;
  }

  int pADCThreshold = 0;
  int pMinClusterSize = 3;
  //Maximum time difference between strips in time sorted cluster (x or y)
  float pDeltaTimeHits = 200;
  //Number of missing strips in strip sorted cluster (x or y)
  int pDeltaStripHits = 2;
  //Maximum time span for total cluster (x or y)
  float pDeltaTimeSpan = 500;
  //Maximum cluster time difference between matching clusters in x and y
  //Cluster time is either calculated with center-of-mass or uTPC method
  float pDeltaTimePlanes = 200;

  NMXClusterer *nmxdata;
};

TEST_F(NMXClustererTest, Run16_line_110168_110323) {
  for (auto hit : Run16) { // replace with UDP receive()
    int result = nmxdata->AnalyzeHits(hit.srs_timestamp, hit.framecounter,
                                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc, hit.adc,
                                      hit.overthreshold);
    if (result == -1) {
      printf("result == -1\n");
      break;
    }
  }
  EXPECT_EQ(0, nmxdata->stats_triggertime_wraps);
  EXPECT_EQ(0, nmxdata->stats_fc_error);
  EXPECT_EQ(0, nmxdata->stats_bcid_tdc_error);
  EXPECT_EQ(nmxdata->getNumClustersX(), 3);
  EXPECT_EQ(nmxdata->getNumClustersY(), 4);
  EXPECT_EQ(nmxdata->getNumClustersXY(), 2);
  //EXPECT_EQ(nmxdata->getNumClustersXY_uTPC(), 2);
}


TEST_F(NMXClustererTest, Run16_Long) {
  for (auto hit : Run16_Long) { // replace with UDP receive()
    int result = nmxdata->AnalyzeHits(hit.srs_timestamp, hit.framecounter,
                                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc, hit.adc,
                                      hit.overthreshold);
    if (result == -1) {
      printf("result == -1\n");
      break;
    }
  }
  EXPECT_EQ(0, nmxdata->stats_triggertime_wraps);
  EXPECT_EQ(0, nmxdata->stats_fc_error);
  EXPECT_EQ(0, nmxdata->stats_bcid_tdc_error);
  EXPECT_EQ(nmxdata->getNumClustersX(), 9110);
  EXPECT_EQ(nmxdata->getNumClustersY(), 11275);
  EXPECT_EQ(nmxdata->getNumClustersXY(), 7302);
  //EXPECT_EQ(nmxdata->getNumClustersXY_uTPC(), 7089);
}

TEST_F(NMXClustererTest, FrameCounterError) {
  EXPECT_EQ(0, nmxdata->stats_fc_error);

  for (auto hit : err_fc_error) {
    nmxdata->AnalyzeHits(hit.srs_timestamp, hit.framecounter, hit.fec,
                         hit.chip_id, hit.channel, hit.bcid, hit.tdc, hit.adc,
                         hit.overthreshold);
  }
  EXPECT_EQ(1, nmxdata->stats_fc_error);
}

TEST_F(NMXClustererTest, BcidTdcError) {
  EXPECT_EQ(0, nmxdata->stats_bcid_tdc_error);

  for (auto hit : err_bcid_tdc_error) {
    nmxdata->AnalyzeHits(hit.srs_timestamp, hit.framecounter, hit.fec,
                         hit.chip_id, hit.channel, hit.bcid, hit.tdc, hit.adc,
                         hit.overthreshold);
  }
  EXPECT_EQ(4, nmxdata->stats_bcid_tdc_error); // Two in X and Two in Y
}

TEST_F(NMXClustererTest, TriggerTimeWraps) {
  EXPECT_EQ(0, nmxdata->stats_triggertime_wraps);

  for (auto hit : err_triggertime_error) {
    nmxdata->AnalyzeHits(hit.srs_timestamp, hit.framecounter, hit.fec,
                         hit.chip_id, hit.channel, hit.bcid, hit.tdc, hit.adc,
                         hit.overthreshold);
  }
  EXPECT_EQ(1, nmxdata->stats_triggertime_wraps);
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
