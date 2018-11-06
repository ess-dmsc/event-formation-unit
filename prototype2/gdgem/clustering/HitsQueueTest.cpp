/** Copyright (C) 2017 European Spallation Source ERIC */

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <gdgem/clustering/HitsQueue.h>
#include <gdgem/NMXConfig.h>
#include <test/TestBase.h>
#include <functional>

//#include <gdgem/clustering/TestDataShort.h>
#include <gdgem/nmx/Readout.h>

#define UNUSED __attribute__((unused))

class HitsQueueTest : public TestBase {
protected:
  NMXConfig opts;
  std::string DataPath;
  std::vector<Readout> readouts;

  std::shared_ptr<HitsQueue> p0;
  std::shared_ptr<HitsQueue> p1;

  virtual void SetUp() {
    DataPath = TEST_DATA_PATH;
    opts = NMXConfig(DataPath + "config.json", "");

    p0 = std::make_shared<HitsQueue>(opts.time_config, opts.clusterer_x.max_time_gap);
    p1 = std::make_shared<HitsQueue>(opts.time_config, opts.clusterer_y.max_time_gap);
  }

  virtual void TearDown() {
  }
};

TEST_F(HitsQueueTest, PrintConfig) {
  MESSAGE() << "Test data config:\n" << opts.debug() << "\n";
}

TEST_F(HitsQueueTest, a1_notrigger) {
  ReadoutFile::read(DataPath + "a00001", readouts);

  for (const auto &r : readouts) {
    auto plane = opts.srs_mappings.get_plane(r);
    EXPECT_EQ(plane, 0);
    p0->store(plane, opts.srs_mappings.get_strip(r),
              r.adc, r.chiptime, r.srs_timestamp);

  }
  EXPECT_EQ(p0->hits().size(), 0);
  p0->sort_and_correct();
  EXPECT_EQ(p0->hits().size(), 0);
  p0->sort_and_correct();
  EXPECT_EQ(p0->hits().size(), 144);
  p0->sort_and_correct();
  EXPECT_EQ(p0->hits().size(), 0);
}

TEST_F(HitsQueueTest, a1_trigger) {
  ReadoutFile::read(DataPath + "a00001", readouts);

  for (const auto &r : readouts) {
    auto plane = opts.srs_mappings.get_plane(r);
    EXPECT_EQ(plane, 0);
    p0->store(plane, opts.srs_mappings.get_strip(r),
              r.adc, r.chiptime, r.srs_timestamp);

  }
  EXPECT_EQ(p0->hits().size(), 0);
  p0->subsequent_trigger(true);
  p0->sort_and_correct();
  EXPECT_EQ(p0->hits().size(), 0);
  p0->subsequent_trigger(true);
  p0->sort_and_correct();
  EXPECT_EQ(p0->hits().size(), 144);
  p0->subsequent_trigger(true);
  p0->sort_and_correct();
  EXPECT_EQ(p0->hits().size(), 0);
}


/// \todo some checks disabled, this is not stricly chronological!!!

//TEST_F(HitsQueueTest, Run16_chronological_no_trigger) {
//  for (auto hit : Run16) {
//    auto chiptime = srstime.chip_time_ns(hit.bcid, hit.tdc, no_offset, unit_slope);
//    queue->store(0,0,0,chiptime);
//  }
//
//  double prevtime{0};
//
//  queue->sort_and_correct();
//  while(queue->hits().size()) {
////    EXPECT_GE(queue->hits().front().time, prevtime);
//    prevtime = queue->hits().front().time;
//    for (const auto &e : queue->hits()) {
//      EXPECT_GE(e.time, prevtime);
//      prevtime = e.time;
//    }
//    queue->sort_and_correct();
//  };
//}

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

//TEST_F(HitsQueueTest, Long_chronological_no_trigger) {
//  for (auto hit : long_data) {
//    auto chiptime = srstime.chip_time_ns(hit.bcid, hit.tdc, no_offset, unit_slope);
//    queue->store(0,0,0,chiptime);
//  }
//
//  double prevtime{0};
//
//  queue->sort_and_correct();
//  while(queue->hits().size()) {
////    EXPECT_GE(queue->hits().front().time, prevtime);
//    prevtime = queue->hits().front().time;
//    for (const auto &e : queue->hits()) {
//      EXPECT_GE(e.time, prevtime);
//      prevtime = e.time;
//    }
//    queue->sort_and_correct();
//  };
//}
//
//TEST_F(HitsQueueTest, Long_chronological_with_trigger) {
//  for (auto hit : long_data) {
//    auto chiptime = srstime.chip_time_ns(hit.bcid, hit.tdc, no_offset, unit_slope);
//    queue->store(0,0,0,chiptime);
//  }
//
//  double prevtime{0};
//
//  queue->subsequent_trigger(true);
//  queue->sort_and_correct();
//  while(queue->hits().size()) {
//    EXPECT_GE(queue->hits().front().time, prevtime);
//    prevtime = queue->hits().front().time;
//    for (const auto &e : queue->hits()) {
////      EXPECT_GE(e.time, prevtime);
//      prevtime = e.time;
//    }
//    queue->subsequent_trigger(true);
//    queue->sort_and_correct();
//  };
//}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
