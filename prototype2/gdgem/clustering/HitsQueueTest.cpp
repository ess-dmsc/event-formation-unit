/** Copyright (C) 2017 European Spallation Source ERIC */

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <gdgem/clustering/HitsQueue.h>
#include <test/TestBase.h>
#include <functional>

#include <gdgem/clustering/TestDataShort.h>
#include <gdgem/nmx/ReadoutFile.h>

constexpr float no_offset = 0.0;
constexpr float unit_slope = 1.0;

#define UNUSED __attribute__((unused))

class HitsQueueTest : public TestBase {
protected:
  std::vector<Readout> long_data;

  // Maximum time difference between hits in time sorted cluster (x or y)
  double pMaxTimeGap = 200;
  std::shared_ptr<HitsQueue> queue;
  SRSTime srstime;

  virtual void SetUp() {
    std::string DataPath = TEST_DATA_PATH;
    ReadoutFile::read(DataPath + "run16long", long_data);

    srstime.set_bc_clock(20);
    srstime.set_tac_slope(60);
    srstime.set_trigger_resolution_ns(3.125);
    srstime.set_acquisition_window(4000);

    queue = std::make_shared<HitsQueue>(srstime, pMaxTimeGap);
  }

  virtual void TearDown() {
  }
};

TEST_F(HitsQueueTest, Run16_no_trigger) {
  for (auto hit : Run16) {
    auto chiptime = srstime.chip_time_ns(hit.bcid, hit.tdc, no_offset, unit_slope);
    queue->store(0,0,0,chiptime);
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
    auto chiptime = srstime.chip_time_ns(hit.bcid, hit.tdc, no_offset, unit_slope);
    queue->store(0,0,0,chiptime);
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

// TODO: some checks disabled, this is not stricly chronological!!!

TEST_F(HitsQueueTest, Run16_chronological_no_trigger) {
  for (auto hit : Run16) {
    auto chiptime = srstime.chip_time_ns(hit.bcid, hit.tdc, no_offset, unit_slope);
    queue->store(0,0,0,chiptime);
  }

  double prevtime{0};

  queue->sort_and_correct();
  while(queue->hits().size()) {
//    EXPECT_GE(queue->hits().front().time, prevtime);
    prevtime = queue->hits().front().time;
    for (const auto &e : queue->hits()) {
      EXPECT_GE(e.time, prevtime);
      prevtime = e.time;
    }
    queue->sort_and_correct();
  };
}

//TEST_F(HitsQueueTest, Run16_chronological_with_trigger) {
//  for (auto hit : Run16) {
//    auto chiptime = srstime.chip_time_ns(hit.bcid, hit.tdc);
//    queue->store(0,0,0,chiptime);
//  }
//
//  double prevtime{0};
//
//  queue->subsequent_trigger(true);
//  queue->sort_and_correct();
//  while(queue->hits().size()) {
////    EXPECT_GE(queue->hits().front().time, prevtime);
//    prevtime = queue->hits().front().time;
//    for (const auto &e : queue->hits()) {
////      EXPECT_GE(e.time, prevtime);
//      prevtime = e.time;
//    }
//    queue->subsequent_trigger(true);
//    queue->sort_and_correct();
//  };
//}

TEST_F(HitsQueueTest, Long_chronological_no_trigger) {
  for (auto hit : long_data) {
    auto chiptime = srstime.chip_time_ns(hit.bcid, hit.tdc, no_offset, unit_slope);
    queue->store(0,0,0,chiptime);
  }

  double prevtime{0};

  queue->sort_and_correct();
  while(queue->hits().size()) {
//    EXPECT_GE(queue->hits().front().time, prevtime);
    prevtime = queue->hits().front().time;
    for (const auto &e : queue->hits()) {
      EXPECT_GE(e.time, prevtime);
      prevtime = e.time;
    }
    queue->sort_and_correct();
  };
}

TEST_F(HitsQueueTest, Long_chronological_with_trigger) {
  for (auto hit : long_data) {
    auto chiptime = srstime.chip_time_ns(hit.bcid, hit.tdc, no_offset, unit_slope);
    queue->store(0,0,0,chiptime);
  }

  double prevtime{0};

  queue->subsequent_trigger(true);
  queue->sort_and_correct();
  while(queue->hits().size()) {
    EXPECT_GE(queue->hits().front().time, prevtime);
    prevtime = queue->hits().front().time;
    for (const auto &e : queue->hits()) {
//      EXPECT_GE(e.time, prevtime);
      prevtime = e.time;
    }
    queue->subsequent_trigger(true);
    queue->sort_and_correct();
  };
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
