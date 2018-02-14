/** Copyright (C) 2017 European Spallation Source ERIC */

#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <gdgem/dg_impl/NMXClusterer.h>
#include <gdgem/dg_impl/TestData.h>
#include <test/TestBase.h>

#define UNUSED __attribute__((unused))

class NMXClustererTest : public TestBase {
protected:
  virtual void SetUp() {  }
  virtual void TearDown() { }

	std::vector<int> pXChips {0, 1, 6, 7};
	std::vector<int> pYChips {10, 11, 14, 15};
	int pTAC = 60;
	int pBC = 20;
	int pAcqWin = 4000;
	int pADCThreshold=0;
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
};

int distributeto(UNUSED int fec, int asic, int channel, int width) {
  int cutoff = 63 - width;
  if ((asic == 10) || (asic == 11 && (channel < cutoff)) )
    return 1;
  if ((asic == 15) || (asic == 14 && (channel >= width)) )
    return 2;
  return 3;
}

TEST_F(NMXClustererTest, Run16_line_110168_110323)
{
	NMXClusterer nmxdata(pBC, pTAC, pAcqWin, pXChips, pYChips, pADCThreshold, pMinClusterSize, pDeltaTimeHits, pDeltaStripHits,pDeltaTimeSpan,pDeltaTimePlanes);

  for (auto hit : Run16_line_110168_110323) { // replace with UDP receive()
		int result = nmxdata.AnalyzeHits(hit.srs_timestamp, hit.framecounter, hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc, hit.adc, hit.overthreshold);
		if (result == -1) {
			printf("result == -1\n");
			break;
		}
	}

	ASSERT_EQ(nmxdata.getNumClustersX(), 3);
	ASSERT_EQ(nmxdata.getNumClustersY(), 4);
	ASSERT_EQ(nmxdata.getNumClustersXY(), 2);
	ASSERT_EQ(nmxdata.getNumClustersXY_uTPC(), 2);
}

TEST_F(NMXClustererTest, GetPlaneID)
{
	NMXClusterer nmxdata(pBC, pTAC, pAcqWin, pXChips, pYChips, pADCThreshold, pMinClusterSize, pDeltaTimeHits, pDeltaStripHits,pDeltaTimeSpan,pDeltaTimePlanes);

  ASSERT_EQ(0, nmxdata.GetPlaneID(0));
  ASSERT_EQ(0, nmxdata.GetPlaneID(1));
  ASSERT_EQ(0, nmxdata.GetPlaneID(6));
  ASSERT_EQ(0, nmxdata.GetPlaneID(7));

  ASSERT_EQ(1, nmxdata.GetPlaneID(10));
  ASSERT_EQ(1, nmxdata.GetPlaneID(11));
  ASSERT_EQ(1, nmxdata.GetPlaneID(14));
  ASSERT_EQ(1, nmxdata.GetPlaneID(15));

  ASSERT_EQ(-1, nmxdata.GetPlaneID(42));
}

TEST_F(NMXClustererTest, GetChannel)
{
	NMXClusterer nmxdata(pBC, pTAC, pAcqWin, pXChips, pYChips, pADCThreshold, pMinClusterSize, pDeltaTimeHits, pDeltaStripHits,pDeltaTimeSpan,pDeltaTimePlanes);

  ASSERT_EQ(0, nmxdata.GetChannel(pXChips, 0, 0));
  ASSERT_EQ(-1, nmxdata.GetChannel(pXChips, 42, 0));
}

TEST_F(NMXClustererTest, FrameCounterError)
{
	NMXClusterer nmxdata(pBC, pTAC, pAcqWin, pXChips, pYChips, pADCThreshold, pMinClusterSize, pDeltaTimeHits, pDeltaStripHits,pDeltaTimeSpan,pDeltaTimePlanes);
  ASSERT_EQ(0, nmxdata.stats_fc_error);

  for (auto hit : err_fc_error) {
    nmxdata.AnalyzeHits(hit.srs_timestamp, hit.framecounter, hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc, hit.adc, hit.overthreshold);
  }
  ASSERT_EQ(1, nmxdata.stats_fc_error);
}

TEST_F(NMXClustererTest, BcidTdcError)
{
	NMXClusterer nmxdata(pBC, pTAC, pAcqWin, pXChips, pYChips, pADCThreshold, pMinClusterSize, pDeltaTimeHits, pDeltaStripHits,pDeltaTimeSpan,pDeltaTimePlanes);
  ASSERT_EQ(0, nmxdata.stats_bcid_tdc_error);

  for (auto hit : err_bcid_tdc_error) {
    nmxdata.AnalyzeHits(hit.srs_timestamp, hit.framecounter, hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc, hit.adc, hit.overthreshold);
  }
  ASSERT_EQ(4, nmxdata.stats_bcid_tdc_error); // Two in X and Two in Y
}

#if 0
TEST_F(NMXClustererTest, TestLoadDistribution)
{
	NMXClusterer nmxdata1(pBC, pTAC, pAcqWin, pXChips, pYChips, pADCThreshold, pMinClusterSize, pDeltaTimeHits, pDeltaStripHits,pDeltaTimeSpan,pDeltaTimePlanes);
  NMXClusterer nmxdata2(pBC, pTAC, pAcqWin, pXChips, pYChips, pADCThreshold, pMinClusterSize, pDeltaTimeHits, pDeltaStripHits,pDeltaTimeSpan,pDeltaTimePlanes);

  for (auto hit : Run16_line_110168_110323) { // replace with UDP receive()
    printf("fec: %d, asic: %d, ch: %d -> %d\n", hit.fec, hit.chip_id, hit.channel, distributeto(hit.fec, hit.chip_id, hit.channel, 10));
    if  (distributeto(hit.fec, hit.chip_id, hit.channel, 10) & 0x01){
      nmxdata1.AnalyzeHits(hit.srs_timestamp, hit.framecounter, hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc, hit.adc, hit.overthreshold);
    }
    if (distributeto(hit.fec, hit.chip_id, hit.channel, 10) & 0x02) {
      nmxdata2.AnalyzeHits(hit.srs_timestamp, hit.framecounter, hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc, hit.adc, hit.overthreshold);
    }
	}
  ASSERT_EQ(nmxdata1.getNumClustersX() + nmxdata2.getNumClustersX(), 3);
  ASSERT_EQ(nmxdata1.getNumClustersY() + nmxdata2.getNumClustersY(), 4);
  ASSERT_EQ(nmxdata1.getNumClustersXY() + nmxdata2.getNumClustersXY(), 2);
  ASSERT_EQ(nmxdata1.getNumClustersXY_uTPC() + nmxdata1.getNumClustersXY_uTPC(), 2);
}
#endif

int main(int argc, char **argv) {
  	testing::InitGoogleTest(&argc, argv);
  	return RUN_ALL_TESTS();
}
