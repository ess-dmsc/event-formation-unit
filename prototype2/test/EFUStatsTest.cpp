/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include "TestBase.h"
#include <common/EFUStats.h>
#include <string>
#include <unistd.h>

class EFUStatsTest : public TestBase {};

TEST_F(EFUStatsTest, Constructor) {

  EFUStats teststats;

  uint64_t *sp = (uint64_t *)&teststats.stats;

  for (unsigned int i = 0; i < sizeof(teststats.stats) / sizeof(uint64_t);
       i++) {
    ASSERT_EQ(*sp, 0);
    sp++;
  }

  sp = (uint64_t *)&teststats.stats_old;

  for (unsigned int i = 0; i < sizeof(teststats.stats_old) / sizeof(uint64_t);
       i++) {
    ASSERT_EQ(*sp, 0);
    sp++;
  }
}

TEST_F(EFUStatsTest, ReportsMask) {
  std::string output;
  EFUStats teststats;

  testing::internal::CaptureStdout();

  sleep(1);
  teststats.report();
  output = testing::internal::GetCapturedStdout();
  ASSERT_EQ(output, "");

  teststats.set_mask(1);
  sleep(1);
  teststats.report();
  teststats.set_mask(2);
  sleep(1);
  teststats.report();
  teststats.set_mask(4);
  sleep(1);
  teststats.report();
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
