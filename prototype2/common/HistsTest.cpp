/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Hists.h>
#include <test/TestBase.h>
#include <math.h>

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
    ASSERT_EQ(hists.bin_width(), pow(2, i));
  }

  // for (unsigned int i = 32; i < 64; i++) {
  //   hists.set_cluster_adc_downshift(i);
  //   ASSERT_EQ(hists.bin_width(), pow(2, 32));
  // }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
