/** Copyright (C) 2017 European Spallation Source ERIC */

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <gdgem/dg_impl/HitsQueue.h>
#include <test/TestBase.h>
#include <functional>

#include <gdgem/dg_impl/TestDataShort.h>
//#include <gdgem/dg_impl/TestDataLong.h>

#define UNUSED __attribute__((unused))

class HitsQueueTest : public TestBase {
protected:
  // Maximum time difference between hits in time sorted cluster (x or y)
  double pMaxTimeGap = 200;
  std::shared_ptr<HitsQueue> queue;
  SRSTime srstime;

  virtual void SetUp() {
    srstime.set_bc_clock(20);
    srstime.set_tac_slope(60);
    srstime.set_trigger_resolution(3.125);
    srstime.set_acquisition_window(4000);

    queue = std::make_shared<HitsQueue>(srstime, pMaxTimeGap);
  }

  virtual void TearDown() {
  }
};

TEST_F(HitsQueueTest, Run16_no_trigger) {
  for (auto hit : Run16) {
    auto chiptime = srstime.chip_time(hit.bcid, hit.tdc);
    queue->store(0,0,chiptime);
  }
  EXPECT_EQ(queue->hits().size(), 0);
  queue->sort_and_correct();
  EXPECT_EQ(queue->hits().size(), 106);
  queue->sort_and_correct();
  EXPECT_EQ(queue->hits().size(), 50);
  queue->sort_and_correct();
  EXPECT_EQ(queue->hits().size(), 0);
}

TEST_F(HitsQueueTest, Run16_with_trigger) {
  for (auto hit : Run16) {
    auto chiptime = srstime.chip_time(hit.bcid, hit.tdc);
    queue->store(0,0,chiptime);
  }
  EXPECT_EQ(queue->hits().size(), 0);
  queue->subsequent_trigger(true);
  queue->sort_and_correct();
  EXPECT_EQ(queue->hits().size(), 123);
  queue->subsequent_trigger(true);
  queue->sort_and_correct();
  EXPECT_EQ(queue->hits().size(), 33);
  queue->subsequent_trigger(true);
  queue->sort_and_correct();
  EXPECT_EQ(queue->hits().size(), 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
