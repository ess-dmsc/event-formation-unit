/** Copyright (C) 2017 European Spallation Source ERIC */

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <gdgem/clustering/HitSorter.h>
#include <gdgem/NMXConfig.h>
#include <test/TestBase.h>
#include <functional>

#include <gdgem/nmx/Readout.h>

using namespace Gem;

class MockClusterer : public AbstractClusterer {
public:
  MockClusterer() {}
  virtual ~MockClusterer() {}

  void cluster(const HitContainer &hits) override
  {
    for (const auto& h :hits)
    {
      if (h.time < prev_time_)
        stats_chrono_errors++;
      prev_time_ = h.time;
    }
    all_hits.insert(all_hits.end(), hits.begin(), hits.end());
  }

  HitContainer all_hits;
  size_t stats_chrono_errors {0};

  double prev_time_{0};
};


class HitSorterTest : public TestBase {
protected:
  NMXConfig opts;
  std::string DataPath;
  std::vector<Readout> readouts;

  uint32_t overflows_x {0};
  uint32_t overflows_y {0};
  uint64_t old_x {0};
  uint64_t old_y {0};

  std::shared_ptr<MockClusterer> mock_x;
  std::shared_ptr<MockClusterer> mock_y;
  std::shared_ptr<HitSorter> sorter_x;
  std::shared_ptr<HitSorter> sorter_y;

  virtual void SetUp() {
    DataPath = TEST_DATA_PATH;
    opts = NMXConfig(DataPath + "/config.json", "");

    mock_x = std::make_shared<MockClusterer>();
    mock_y = std::make_shared<MockClusterer>();
    sorter_x = std::make_shared<HitSorter>(opts.time_config, opts.srs_mappings,
                                           opts.clusterer_x.hit_adc_threshold,
                                           opts.clusterer_x.max_time_gap);
    sorter_y = std::make_shared<HitSorter>(opts.time_config, opts.srs_mappings,
                                           opts.clusterer_y.hit_adc_threshold,
                                           opts.clusterer_y.max_time_gap);
    sorter_x->clusterer = mock_x;
    sorter_y->clusterer = mock_y;
  }

  virtual void TearDown() {
  }

  void add_readouts() {
    for (const auto& readout : readouts) {
      auto plane = opts.srs_mappings.get_plane(readout);
      EXPECT_LT(plane, 2) << "BAD PLANE"
                          << " fec:" << int(readout.fec)
                          << " chip:" << int(readout.chip_id) << "\n";

      if (plane == 0) {
        if (readout.srs_timestamp < old_x)
          overflows_x++;
        old_x = readout.srs_timestamp;
        sorter_x->insert(readout);
      }
      if (plane == 1) {
        if (readout.srs_timestamp < old_y)
          overflows_y++;
        old_y = readout.srs_timestamp;
        sorter_y->insert(readout);
      }
    }

    EXPECT_EQ(overflows_x, 0);
    EXPECT_EQ(overflows_y, 0);
  }

  void test_stats(size_t triggers_x, size_t triggers_y, bool test_subs) {
    EXPECT_EQ(triggers_x, sorter_x->stats_trigger_count);
    EXPECT_EQ(triggers_y, sorter_y->stats_trigger_count);

    if (test_subs) {
      EXPECT_EQ(0, sorter_x->stats_subsequent_triggers);
      EXPECT_EQ(0, sorter_y->stats_subsequent_triggers);
    }

    EXPECT_EQ(0, mock_x->stats_chrono_errors);
    EXPECT_EQ(0, mock_y->stats_chrono_errors);
  }


};

TEST_F(HitSorterTest, Constructor) {
  ASSERT_EQ(0, sorter_x->stats_trigger_count);
  ASSERT_EQ(0, sorter_y->stats_trigger_count);
  ASSERT_EQ(0, sorter_x->stats_subsequent_triggers);
  ASSERT_EQ(0, sorter_y->stats_subsequent_triggers);
  ASSERT_EQ(true, mock_x->empty());
  ASSERT_EQ(true, mock_y->empty());
}

TEST_F(HitSorterTest, a1) {
  ReadoutFile::read(DataPath + "/readouts/a00001", readouts);
  EXPECT_EQ(readouts.size(), 144);

  add_readouts();
  test_stats(15, 0, true);
}

TEST_F(HitSorterTest, a1_chrono) {
  ReadoutFile::read(DataPath + "/readouts/a00001", readouts);
  EXPECT_EQ(readouts.size(), 144);

  add_readouts();
  test_stats(15, 0, true);

  // flush, but must it be with trigger?
  sorter_x->flush();
  sorter_y->flush();

  test_stats(15, 0, true);

  EXPECT_EQ(144, mock_x->all_hits.size());
  EXPECT_EQ(0, mock_y->all_hits.size());
  EXPECT_EQ(readouts.size(), (mock_x->all_hits.size() + mock_y->all_hits.size()));
}

TEST_F(HitSorterTest, a10) {
  ReadoutFile::read(DataPath + "/readouts/a00010", readouts);
  EXPECT_EQ(readouts.size(), 920);

  add_readouts();

  EXPECT_EQ(74, sorter_x->stats_trigger_count);
  EXPECT_EQ(58, sorter_y->stats_trigger_count);

  EXPECT_EQ(0, sorter_x->stats_subsequent_triggers);
  EXPECT_EQ(0, sorter_y->stats_subsequent_triggers);
}

TEST_F(HitSorterTest, a10_chrono) {
  ReadoutFile::read(DataPath + "/readouts/a00010", readouts);
  EXPECT_EQ(readouts.size(), 920);

  add_readouts();
  test_stats(74, 58, true);

  // flush, but must it be with trigger?
  sorter_x->flush();
  sorter_y->flush();

  test_stats(74, 58, true);

  EXPECT_EQ(558, mock_x->all_hits.size());
  EXPECT_EQ(362, mock_y->all_hits.size());
  EXPECT_EQ(readouts.size(), (mock_x->all_hits.size() + mock_y->all_hits.size()));
}

TEST_F(HitSorterTest, a100) {
  ReadoutFile::read(DataPath + "/readouts/a00100", readouts);
  EXPECT_EQ(readouts.size(), 126590);

  add_readouts();

  test_stats(3974, 3469, false);

}

TEST_F(HitSorterTest, a100_chrono) {
  ReadoutFile::read(DataPath + "/readouts/a00100", readouts);
  EXPECT_EQ(readouts.size(), 126590);

  add_readouts();

  test_stats(3974, 3469, false);

  // flush, but must it be with trigger?
  sorter_x->flush();
  sorter_y->flush();

  test_stats(3974, 3469, false);

  EXPECT_EQ(84162, mock_x->all_hits.size());
  EXPECT_EQ(42428, mock_y->all_hits.size());
  EXPECT_EQ(readouts.size(), (mock_x->all_hits.size() + mock_y->all_hits.size()));
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
