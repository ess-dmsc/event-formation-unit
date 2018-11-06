/** Copyright (C) 2017 European Spallation Source ERIC */

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <gdgem/clustering/HitsQueue.h>
#include <gdgem/NMXConfig.h>
#include <test/TestBase.h>
#include <functional>

#include <gdgem/nmx/Readout.h>

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
  EXPECT_EQ(readouts.size(), 144);

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
  EXPECT_EQ(readouts.size(), 144);

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

TEST_F(HitsQueueTest, a1_chronological_no_trigger) {
  ReadoutFile::read(DataPath + "a00001", readouts);
  EXPECT_EQ(readouts.size(), 144);

  for (const auto &r : readouts) {
    auto plane = opts.srs_mappings.get_plane(r);
    EXPECT_EQ(plane, 0);
    p0->store(plane, opts.srs_mappings.get_strip(r),
              r.adc, r.chiptime, r.srs_timestamp);

  }

  double prevtime{0};
  p0->sort_and_correct();
  p0->sort_and_correct();
  while (p0->hits().size()) {
    EXPECT_GE(p0->hits().front().time, prevtime);
    prevtime = p0->hits().front().time;
    for (const auto &e : p0->hits()) {
      EXPECT_GE(e.time, prevtime);
      prevtime = e.time;
    }
    p0->sort_and_correct();
  }
}

TEST_F(HitsQueueTest, a10_notrigger) {
  ReadoutFile::read(DataPath + "a00010", readouts);
  EXPECT_EQ(readouts.size(), 920);

  for (const auto &r : readouts) {
    auto plane = opts.srs_mappings.get_plane(r);
    EXPECT_LT(plane, 2) << "fec:" << int(r.fec) << " chip:" << int(r.chip_id) << "\n";
    if (plane == 0)
      p0->store(plane, opts.srs_mappings.get_strip(r),
                r.adc, r.chiptime, r.srs_timestamp);

    if (plane == 1)
      p1->store(plane, opts.srs_mappings.get_strip(r),
                r.adc, r.chiptime, r.srs_timestamp);
  }
  EXPECT_EQ(p0->hits().size(), 0);
  EXPECT_EQ(p1->hits().size(), 0);
  p0->sort_and_correct();
  p1->sort_and_correct();
  EXPECT_EQ(p0->hits().size(), 0);
  EXPECT_EQ(p1->hits().size(), 0);
  p0->sort_and_correct();
  p1->sort_and_correct();
  EXPECT_EQ(p0->hits().size(), 558);
  EXPECT_EQ(p1->hits().size(), 362);
  p0->sort_and_correct();
  p1->sort_and_correct();
  EXPECT_EQ(p0->hits().size(), 0);
  EXPECT_EQ(p1->hits().size(), 0);
}


TEST_F(HitsQueueTest, a10_trigger) {
  ReadoutFile::read(DataPath + "a00010", readouts);
  EXPECT_EQ(readouts.size(), 920);

  for (const auto &r : readouts) {
    auto plane = opts.srs_mappings.get_plane(r);
    EXPECT_LT(plane, 2) << "fec:" << int(r.fec) << " chip:" << int(r.chip_id) << "\n";
    if (plane == 0)
      p0->store(plane, opts.srs_mappings.get_strip(r),
                r.adc, r.chiptime, r.srs_timestamp);

    if (plane == 1)
      p1->store(plane, opts.srs_mappings.get_strip(r),
                r.adc, r.chiptime, r.srs_timestamp);
  }
  EXPECT_EQ(p0->hits().size(), 0);
  EXPECT_EQ(p1->hits().size(), 0);
  p0->subsequent_trigger(true);
  p1->subsequent_trigger(true);
  p0->sort_and_correct();
  p1->sort_and_correct();
  EXPECT_EQ(p0->hits().size(), 0);
  EXPECT_EQ(p1->hits().size(), 0);
  p0->subsequent_trigger(true);
  p1->subsequent_trigger(true);
  p0->sort_and_correct();
  p1->sort_and_correct();
  EXPECT_EQ(p0->hits().size(), 558);
  EXPECT_EQ(p1->hits().size(), 362);
  p0->subsequent_trigger(true);
  p1->subsequent_trigger(true);
  p0->sort_and_correct();
  p1->sort_and_correct();
  EXPECT_EQ(p0->hits().size(), 0);
  EXPECT_EQ(p1->hits().size(), 0);
}

TEST_F(HitsQueueTest, a10_chronological_no_trigger) {
  ReadoutFile::read(DataPath + "a00010", readouts);
  EXPECT_EQ(readouts.size(), 920);

  for (const auto &r : readouts) {
    auto plane = opts.srs_mappings.get_plane(r);
    EXPECT_LT(plane, 2) << "fec:" << int(r.fec) << " chip:" << int(r.chip_id) << "\n";
    if (plane == 0)
      p0->store(plane, opts.srs_mappings.get_strip(r),
                r.adc, r.chiptime, r.srs_timestamp);

    if (plane == 1)
      p1->store(plane, opts.srs_mappings.get_strip(r),
                r.adc, r.chiptime, r.srs_timestamp);
  }

  double prevtime{0};
  size_t count {0};
  while(count < 558) {
    p0->sort_and_correct();
    if (p0->hits().size()) {
      EXPECT_GE(p0->hits().front().time, prevtime);
    }
    for (const auto &e : p0->hits()) {
      EXPECT_GE(e.time, prevtime);
      prevtime = e.time;
      count++;
    }
  }

  prevtime = 0;
  count = 0;
  while(count < 362) {
    p1->sort_and_correct();
    if (p1->hits().size()) {
      EXPECT_GE(p1->hits().front().time, prevtime);
    }
    for (const auto &e : p1->hits()) {
      EXPECT_GE(e.time, prevtime);
      prevtime = e.time;
      count++;
    }
  }
}

TEST_F(HitsQueueTest, a100_notrigger) {
  ReadoutFile::read(DataPath + "a00100", readouts);
  EXPECT_EQ(readouts.size(), 126590);

  for (const auto &r : readouts) {
    auto plane = opts.srs_mappings.get_plane(r);
    EXPECT_LT(plane, 2) << "fec:" << int(r.fec) << " chip:" << int(r.chip_id) << "\n";
    if (plane == 0)
      p0->store(plane, opts.srs_mappings.get_strip(r),
                r.adc, r.chiptime, r.srs_timestamp);

    if (plane == 1)
      p1->store(plane, opts.srs_mappings.get_strip(r),
                r.adc, r.chiptime, r.srs_timestamp);
  }
  EXPECT_EQ(p0->hits().size(), 0);
  EXPECT_EQ(p1->hits().size(), 0);
  p0->sort_and_correct();
  p1->sort_and_correct();
  EXPECT_EQ(p0->hits().size(), 0);
  EXPECT_EQ(p1->hits().size(), 0);
  p0->sort_and_correct();
  p1->sort_and_correct();
  EXPECT_EQ(p0->hits().size(), 84162);
  EXPECT_EQ(p1->hits().size(), 42428);
  p0->sort_and_correct();
  p1->sort_and_correct();
  EXPECT_EQ(p0->hits().size(), 0);
  EXPECT_EQ(p1->hits().size(), 0);
}

TEST_F(HitsQueueTest, a100_trigger) {
  ReadoutFile::read(DataPath + "a00100", readouts);
  EXPECT_EQ(readouts.size(), 126590);

  for (const auto &r : readouts) {
    auto plane = opts.srs_mappings.get_plane(r);
    EXPECT_LT(plane, 2) << "fec:" << int(r.fec) << " chip:" << int(r.chip_id) << "\n";
    if (plane == 0)
      p0->store(plane, opts.srs_mappings.get_strip(r),
                r.adc, r.chiptime, r.srs_timestamp);

    if (plane == 1)
      p1->store(plane, opts.srs_mappings.get_strip(r),
                r.adc, r.chiptime, r.srs_timestamp);
  }
  EXPECT_EQ(p0->hits().size(), 0);
  EXPECT_EQ(p1->hits().size(), 0);
  p0->subsequent_trigger(true);
  p1->subsequent_trigger(true);
  p0->sort_and_correct();
  p1->sort_and_correct();
  EXPECT_EQ(p0->hits().size(), 0);
  EXPECT_EQ(p1->hits().size(), 0);
  p0->subsequent_trigger(true);
  p1->subsequent_trigger(true);
  p0->sort_and_correct();
  p1->sort_and_correct();
  EXPECT_EQ(p0->hits().size(), 84162);
  EXPECT_EQ(p1->hits().size(), 42428);
  p0->subsequent_trigger(true);
  p1->subsequent_trigger(true);
  p0->sort_and_correct();
  p1->sort_and_correct();
  EXPECT_EQ(p0->hits().size(), 0);
  EXPECT_EQ(p1->hits().size(), 0);
}

TEST_F(HitsQueueTest, a100_chronological_no_trigger) {
  ReadoutFile::read(DataPath + "a00100", readouts);
  EXPECT_EQ(readouts.size(), 126590);

  for (const auto &r : readouts) {
    auto plane = opts.srs_mappings.get_plane(r);
    EXPECT_LT(plane, 2) << "fec:" << int(r.fec) << " chip:" << int(r.chip_id) << "\n";
    if (plane == 0)
      p0->store(plane, opts.srs_mappings.get_strip(r),
                r.adc, r.chiptime, r.srs_timestamp);

    if (plane == 1)
      p1->store(plane, opts.srs_mappings.get_strip(r),
                r.adc, r.chiptime, r.srs_timestamp);
  }

  double prevtime{0};
  size_t count {0};
  while(count < 84162) {
    p0->sort_and_correct();
    if (p0->hits().size()) {
      EXPECT_GE(p0->hits().front().time, prevtime);
    }
    for (const auto &e : p0->hits()) {
      EXPECT_GE(e.time, prevtime);
      prevtime = e.time;
      count++;
    }
  }

  prevtime = 0;
  count = 0;
  while(count < 42428) {
    p1->sort_and_correct();
    if (p1->hits().size()) {
      EXPECT_GE(p1->hits().front().time, prevtime);
    }
    for (const auto &e : p1->hits()) {
      EXPECT_GE(e.time, prevtime);
      prevtime = e.time;
      count++;
    }
  }
}
int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
