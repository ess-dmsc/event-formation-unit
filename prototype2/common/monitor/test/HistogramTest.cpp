/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/monitor/Histogram.h>
#include <test/TestBase.h>
#include <cmath>

class HistsTest : public TestBase {};

TEST_F(HistsTest, Constructor) {
  Hists hists(64, 4096);
  ASSERT_EQ(hists.bin_width(), 1);
  ASSERT_TRUE(hists.isEmpty());
  ASSERT_EQ(hists.hit_count(), 0);
  ASSERT_EQ(hists.cluster_count(), 0);
}

TEST_F(HistsTest, SetBinWidth) {
  Hists hists(64, 4096);
  for (unsigned int i = 0; i < 32; i++) {
    hists.set_cluster_adc_downshift(i);
    ASSERT_EQ(hists.bin_width(), std::pow(2, i));
  }

  // for (unsigned int i = 32; i < 64; i++) {
  //   hists.set_cluster_adc_downshift(i);
  //   ASSERT_EQ(hists.bin_width(), pow(2, 32));
  // }
}

TEST_F(HistsTest, BinCluster) {
  Hists hists(64, 4096);
  hists.bincluster(0);
  ASSERT_EQ(hists.cluster_count(), 0);

  for (unsigned int i = 1; i < 4096; i++) {
    hists.bincluster(i);
    ASSERT_EQ(hists.cluster_count(), i);
    ASSERT_EQ(hists.cluster_adc_hist[i], 1);
  }
}

TEST_F(HistsTest, BinClusterWDownshift) {
  Hists hists(64, 4096);
  hists.set_cluster_adc_downshift(1);
  for (unsigned int i = 1; i < 4096; i++) {
    hists.bincluster(i);
    ASSERT_EQ(hists.cluster_count(), i);
  }
  for (unsigned int i = 1; i < 2048; i++) {
    ASSERT_EQ(hists.cluster_adc_hist[i], 2);
  }
  for (unsigned int i = 2048; i < 4096; i++) {
    ASSERT_EQ(hists.cluster_adc_hist[i], 0);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
