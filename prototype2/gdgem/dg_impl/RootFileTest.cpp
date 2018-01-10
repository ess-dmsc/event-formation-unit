/** Copyright (C) 2017 European Spallation Source ERIC */

#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <gdgem/dg_impl/RootFile.h>
#include <gdgem/dg_impl/TestData.h>
#include <test/TestBase.h>

class RootFileTest : public TestBase {
protected:
  virtual void SetUp() {  }
  virtual void TearDown() { }
};

TEST_F(RootFileTest, Run16_1_to_16)
{
	std::vector<int> xchips {0, 1, 6, 7};
	std::vector<int> ychips {10, 11, 14, 15};
	int tac = 125;
	int bc = 20;
	int acqWin = 7800;
	std::vector<int> xChips, yChips;
	for (auto chip : xchips)
	  xChips.push_back(chip);
	for (auto chip : ychips)
	  yChips.push_back(chip);

	std::string readout = "GEM";
  int threshold=0;
  int clusterSize = 3;
	bool vFound=true;
	unsigned int vStart = 0;
	unsigned int vEnd = 300;

	RootFile nmxdata(bc, tac, acqWin, xChips, yChips, readout, vFound, vStart, vEnd, threshold, clusterSize);

  for (auto hit : Run16_1_to_16) { // replace with UDP receive()
		int result = nmxdata.AnalyzeHitData(hit.srs_timestamp, hit.framecounter, hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc, hit.adc, hit.overthreshold);
		if (result == -1) {
			printf("result == -1\n");
			break;
		}
	}

  ASSERT_EQ(nmxdata.getnCLinX(), 8);
  ASSERT_EQ(nmxdata.getnCLinY(), 5);
  ASSERT_EQ(nmxdata.getnCLinXY(), 4);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
