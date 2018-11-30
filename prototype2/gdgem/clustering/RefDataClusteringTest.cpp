/** Copyright (C) 2017 European Spallation Source ERIC */

#include <gdgem/nmx/Readout.h>
#include <common/clustering/GapClusterer.h>
#include <common/clustering/GapMatcher.h>

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <test/TestBase.h>
#include <gdgem/NMXConfig.h>
#include <functional>

using namespace Gem;

static constexpr size_t min_cluster_size {3};

class HitSorter {
 public:
  HitSorter(SRSTime time, SRSMappings chips) :
      pTime(time), pChips(chips)
  {}

  void insert(const Readout &readout) {
    buffer.push_back(Hit());
    auto &e = buffer.back();
    e.plane = pChips.get_plane(readout);
    e.weight = readout.adc;
    e.coordinate = pChips.get_strip(readout);
    e.time = readout.srs_timestamp + static_cast<uint64_t>(readout.chiptime);
    if (readout.srs_timestamp < prev_srs_time)
      srs_overflows++;
    prev_srs_time = readout.srs_timestamp;
    if (readout.chiptime < 0)
      negative_chip_times++;
  }

  void flush() {
    std::sort(buffer.begin(), buffer.end(),
              [](const Hit &e1, const Hit &e2) {
                return e1.time < e2.time;
              });
    if (clusterer)
      clusterer->cluster(buffer);
    buffer.clear();
    if (clusterer)
      clusterer->flush();
  }

  std::shared_ptr<AbstractClusterer> clusterer;

  HitContainer buffer;
  uint64_t prev_srs_time {0};
  size_t srs_overflows{0};
  size_t negative_chip_times{0};

 private:
  SRSTime pTime;
  SRSMappings pChips;
};

class DoroClustererTest : public TestBase {
protected:
  NMXConfig opts;
  std::string DataPath;
  std::vector<Readout> readouts;

  std::shared_ptr<HitSorter> sorter_x;
  std::shared_ptr<HitSorter> sorter_y;

  std::shared_ptr<AbstractMatcher> matcher;

  virtual void SetUp() {
    DataPath = TEST_DATA_PATH;
    opts = NMXConfig(DataPath + "/config.json", "");

    sorter_x =
        std::make_shared<HitSorter>(opts.time_config, opts.srs_mappings);
    sorter_y =
        std::make_shared<HitSorter>(opts.time_config, opts.srs_mappings);

    sorter_x->clusterer =
        std::make_shared<GapClusterer>(opts.clusterer_x.max_time_gap,
                                       opts.clusterer_x.max_strip_gap);
    sorter_y->clusterer =
        std::make_shared<GapClusterer>(opts.clusterer_y.max_time_gap,
                                       opts.clusterer_y.max_strip_gap);

    matcher = std::make_shared<GapMatcher>(
        opts.time_config.acquisition_window()*5,
        opts.matcher_max_delta_time);
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
        sorter_x->insert(readout);
      }
      if (plane == 1) {
        sorter_y->insert(readout);
      }
    }
  }

  void planes_test(size_t expected_x, size_t expected_y) {
    EXPECT_EQ(readouts.size(), expected_x + expected_y);
    EXPECT_EQ(sorter_x->buffer.size(), expected_x);
    EXPECT_EQ(sorter_y->buffer.size(), expected_y);
  }

  void test_plane(std::shared_ptr<AbstractClusterer> clusterer,
                  size_t expected_total,
                  size_t expected_filtered,
                  size_t min_cluster_size) {
    EXPECT_EQ(clusterer->stats_cluster_count, expected_total);
    size_t count_x{0};
    for (const auto& c : clusterer->clusters)
    {
//      MESSAGE() << c.debug(true) << "\n";
//      MESSAGE() << c.visualize(2) << "\n";
      if (c.hit_count() >= min_cluster_size)
        count_x++;
    }
    EXPECT_EQ(count_x, expected_filtered);
  }

};

TEST_F(DoroClustererTest, PrintConfig) {
  MESSAGE() << "Test data config:\n" << opts.debug() << "\n";
}

TEST_F(DoroClustererTest, a1) {
  ReadoutFile::read(DataPath + "/readouts/a00001", readouts);
  EXPECT_EQ(readouts.size(), 144);

  add_readouts();

  planes_test(144, 0);

  sorter_x->flush();
  sorter_y->flush();

  EXPECT_EQ(sorter_x->srs_overflows, 0);
  EXPECT_EQ(sorter_y->srs_overflows, 0);

  EXPECT_EQ(sorter_x->negative_chip_times, 0);
  EXPECT_EQ(sorter_y->negative_chip_times, 0);

  test_plane(sorter_x->clusterer, 22, 20, min_cluster_size);
  test_plane(sorter_y->clusterer, 0, 0, min_cluster_size);

  matcher->insert(0, sorter_x->clusterer->clusters);
  matcher->insert(1, sorter_y->clusterer->clusters);
  matcher->match(true);
  EXPECT_EQ(matcher->stats_event_count, 15);
  EXPECT_EQ(matcher->matched_events.size(), 15);
}

TEST_F(DoroClustererTest, a10) {
  ReadoutFile::read(DataPath + "/readouts/a00010", readouts);
  EXPECT_EQ(readouts.size(), 920);

  add_readouts();

  planes_test(558, 362);

  sorter_x->flush();
  sorter_y->flush();

  EXPECT_EQ(sorter_x->srs_overflows, 0);
  EXPECT_EQ(sorter_y->srs_overflows, 0);

  EXPECT_EQ(sorter_x->negative_chip_times, 0);
  EXPECT_EQ(sorter_y->negative_chip_times, 0);

  test_plane(sorter_x->clusterer, 100, 96, min_cluster_size);
  test_plane(sorter_y->clusterer, 73, 68, min_cluster_size);

  matcher->insert(0, sorter_x->clusterer->clusters);
  matcher->insert(1, sorter_y->clusterer->clusters);
  matcher->match(true);
  EXPECT_EQ(matcher->stats_event_count, 101);
  EXPECT_EQ(matcher->matched_events.size(), 101);

//  for (const auto& e : matcher->matched_events) {
//    MESSAGE() << e.visualize(3) << "\n";
//  }
}

TEST_F(DoroClustererTest, a100) {
  ReadoutFile::read(DataPath + "/readouts/a00100", readouts);
  EXPECT_EQ(readouts.size(), 126590);

  add_readouts();

  planes_test(84162, 42428);

  sorter_x->flush();
  sorter_y->flush();

  EXPECT_EQ(sorter_x->srs_overflows, 0);
  EXPECT_EQ(sorter_y->srs_overflows, 0);

  EXPECT_EQ(sorter_x->negative_chip_times, 12);
  EXPECT_EQ(sorter_y->negative_chip_times, 0);

  test_plane(sorter_x->clusterer, 19565, 19003, min_cluster_size);
  test_plane(sorter_y->clusterer, 10312, 9737, min_cluster_size);

  matcher->insert(0, sorter_x->clusterer->clusters);
  matcher->insert(1, sorter_y->clusterer->clusters);
  matcher->match(true);
  EXPECT_EQ(matcher->stats_event_count, 20224);
  EXPECT_EQ(matcher->matched_events.size(), 20224);
}

TEST_F(DoroClustererTest, a1000) {
  ReadoutFile::read(DataPath + "/readouts/a01000", readouts);
  EXPECT_EQ(readouts.size(), 1416666);

  add_readouts();

  planes_test(934684, 481982);

  sorter_x->flush();
  sorter_y->flush();

  EXPECT_EQ(sorter_x->srs_overflows, 0);
  EXPECT_EQ(sorter_y->srs_overflows, 0);

  EXPECT_EQ(sorter_x->negative_chip_times, 59);
  EXPECT_EQ(sorter_y->negative_chip_times, 43);


  test_plane(sorter_x->clusterer, 217126, 211247, min_cluster_size);
  test_plane(sorter_y->clusterer, 116771, 109826, min_cluster_size);

  matcher->insert(0, sorter_x->clusterer->clusters);
  matcher->insert(1, sorter_y->clusterer->clusters);
  matcher->match(true);
  EXPECT_EQ(matcher->stats_event_count, 226491);
  EXPECT_EQ(matcher->matched_events.size(), 226491);
}

TEST_F(DoroClustererTest, a10000) {
  ReadoutFile::read(DataPath + "/readouts/a10000", readouts);
  EXPECT_EQ(readouts.size(), 14293164);

  add_readouts();

  planes_test(9423281, 4869883);

  sorter_x->flush();
  sorter_y->flush();

  EXPECT_EQ(sorter_x->srs_overflows, 0);
  EXPECT_EQ(sorter_y->srs_overflows, 0);

  EXPECT_EQ(sorter_x->negative_chip_times, 665);
  EXPECT_EQ(sorter_y->negative_chip_times, 281);

  test_plane(sorter_x->clusterer, 2183659, 2125209, min_cluster_size);
  test_plane(sorter_y->clusterer, 1179938, 1111857, min_cluster_size);

  matcher->insert(0, sorter_x->clusterer->clusters);
  matcher->insert(1, sorter_y->clusterer->clusters);
  matcher->match(true);
  EXPECT_EQ(matcher->stats_event_count, 2285514);
  EXPECT_EQ(matcher->matched_events.size(), 2285514);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
