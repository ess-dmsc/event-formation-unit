/** Copyright (C) 2019 European Spallation Source ERIC */

#include <common/monitor/DynamicHist.h>
#include <test/TestBase.h>
//#include <cmath>

class DynamicHistTest : public TestBase {};

TEST_F(DynamicHistTest, Constructor) {
  DynamicHist histogram;
  ASSERT_TRUE(histogram.empty());
}

TEST_F(DynamicHistTest, NotEmpty) {
  DynamicHist histogram;
  histogram.bin(0);
  ASSERT_FALSE(histogram.empty());
  ASSERT_EQ(histogram.hist.size(), 1);
  histogram.bin(0);
  ASSERT_EQ(histogram.hist.size(), 1);
  histogram.bin(1);
  ASSERT_EQ(histogram.hist.size(), 2);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
