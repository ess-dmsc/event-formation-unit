/** Copyright (C) 2017 European Spallation Source ERIC */

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <gdgem/clustering/HitsQueue.h>
#include <gdgem/NMXConfig.h>
#include <test/TestBase.h>
#include <functional>

#include <gdgem/nmx/Readout.h>

using namespace Gem;

class HitsQueueTest : public TestBase {
protected:
  NMXConfig opts;
  std::string DataPath;
  std::vector<Readout> readouts;

  std::shared_ptr<HitsQueue> p0;
  std::shared_ptr<HitsQueue> p1;

  virtual void SetUp() {
    DataPath = TEST_DATA_PATH;
    opts = NMXConfig(DataPath + "/config.json", "");

    p0 = std::make_shared<HitsQueue>(opts.time_config, opts.clusterer_x.max_time_gap);
    p1 = std::make_shared<HitsQueue>(opts.time_config, opts.clusterer_y.max_time_gap);
  }

  virtual void TearDown() {
  }

  void add_readouts() {
    for (const auto &r : readouts) {
      auto plane = opts.srs_mappings.get_plane(r);
      EXPECT_LT(plane, 2) << "BAD PLANE"
                          << " fec:" << int(r.fec)
                          << " chip:" << int(r.chip_id) << "\n";
      if (plane == 0)
        p0->store(plane, opts.srs_mappings.get_strip(r),
                  r.adc, r.chiptime, r.srs_timestamp);

      if (plane == 1)
        p1->store(plane, opts.srs_mappings.get_strip(r),
                  r.adc, r.chiptime, r.srs_timestamp);
    }
  }

  void regular_test(size_t expected_x, size_t expected_y, bool subs_trigger) {
    EXPECT_EQ(readouts.size(), expected_x + expected_y);
    add_readouts();

    EXPECT_EQ(p0->hits().size(), 0);
    EXPECT_EQ(p1->hits().size(), 0);
    if (subs_trigger) {
      p0->subsequent_trigger(true);
      p1->subsequent_trigger(true);
    }
    p0->sort_and_correct();
    p1->sort_and_correct();
    EXPECT_EQ(p0->hits().size(), 0);
    EXPECT_EQ(p1->hits().size(), 0);
    if (subs_trigger) {
      p0->subsequent_trigger(true);
      p1->subsequent_trigger(true);
    }
    p0->sort_and_correct();
    p1->sort_and_correct();
    EXPECT_EQ(p0->hits().size(), expected_x);
    EXPECT_EQ(p1->hits().size(), expected_y);
    if (subs_trigger) {
      p0->subsequent_trigger(true);
      p1->subsequent_trigger(true);
    }
    p0->sort_and_correct();
    p1->sort_and_correct();
    EXPECT_EQ(p0->hits().size(), 0);
    EXPECT_EQ(p1->hits().size(), 0);
  }

  void chronological_test_x(size_t expected_size)
  {
    double prevtime{0};
    size_t count{0};
    while (count < expected_size) {
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
  }

  void chronological_test_y(size_t expected_size)
  {
    double prevtime{0};
    size_t count{0};
    while (count < expected_size) {
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

};

TEST_F(HitsQueueTest, PrintConfig) {
  MESSAGE() << "Test data config:\n" << opts.debug() << "\n";
}

TEST_F(HitsQueueTest, a1_notrigger) {
  ReadoutFile::read(DataPath + "/readouts/a00001", readouts);
  regular_test(144,0, false);
}

TEST_F(HitsQueueTest, a1_trigger) {
  ReadoutFile::read(DataPath + "/readouts/a00001", readouts);
  regular_test(144,0, true);
}

TEST_F(HitsQueueTest, a1_chronological_no_trigger) {
  ReadoutFile::read(DataPath + "/readouts/a00001", readouts);
  add_readouts();
  chronological_test_x(144);
  chronological_test_y(0);
}

TEST_F(HitsQueueTest, a10_notrigger)
{
  ReadoutFile::read(DataPath + "/readouts/a00010", readouts);
  regular_test(558, 362, false);
}

TEST_F(HitsQueueTest, a10_trigger) {
  ReadoutFile::read(DataPath + "/readouts/a00010", readouts);
  regular_test(558, 362, true);
}

TEST_F(HitsQueueTest, a10_chronological_no_trigger) {
  ReadoutFile::read(DataPath + "/readouts/a00010", readouts);
  add_readouts();
  chronological_test_x(558);
  chronological_test_y(362);
}

TEST_F(HitsQueueTest, a100_notrigger) {
  ReadoutFile::read(DataPath + "/readouts/a00100", readouts);
  regular_test(84162, 42428, false);
}

TEST_F(HitsQueueTest, a100_trigger) {
  ReadoutFile::read(DataPath + "/readouts/a00100", readouts);
  regular_test(84162, 42428, true);
}

TEST_F(HitsQueueTest, a100_chronological_no_trigger) {
  ReadoutFile::read(DataPath + "/readouts/a00100", readouts);
  add_readouts();
  chronological_test_x(84162);
  chronological_test_y(42428);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
