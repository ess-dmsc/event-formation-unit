/** Copyright (C) 2017 European Spallation Source ERIC */

#include <gdgem/nmx/Readout.h>
#include <gdgem/clustering/HitSorter.h>
#include <common/clustering/GapClusterer.h>

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <test/TestBase.h>
#include <gdgem/NMXConfig.h>
#include <functional>

using namespace Gem;

class DoroClustererTest : public TestBase {
protected:
  NMXConfig opts;
  std::string DataPath;
  std::vector<Readout> readouts;

  std::shared_ptr<HitSorter> sorter_x;
  std::shared_ptr<HitSorter> sorter_y;

  virtual void SetUp() {
    DataPath = TEST_DATA_PATH;
    opts = NMXConfig(DataPath + "/config.json", "");

    sorter_x = std::make_shared<HitSorter>(opts.time_config, opts.srs_mappings,
                                           opts.clusterer_x.hit_adc_threshold,
                                           opts.clusterer_x.max_time_gap);
    sorter_y = std::make_shared<HitSorter>(opts.time_config, opts.srs_mappings,
                                           opts.clusterer_y.hit_adc_threshold,
                                           opts.clusterer_y.max_time_gap);

    sorter_x->clusterer =
        std::make_shared<GapClusterer>(opts.clusterer_x.max_time_gap,
            opts.clusterer_x.max_strip_gap);
    sorter_y->clusterer =
        std::make_shared<GapClusterer>(opts.clusterer_y.max_time_gap,
            opts.clusterer_y.max_strip_gap);
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

  void test_plane(std::shared_ptr<AbstractClusterer> clusterer,
                  size_t expected_total,
                  size_t expected_filtered,
                  size_t min_cluster_size) {
    EXPECT_EQ(clusterer->stats_cluster_count, expected_total);
    size_t count_x{0};
    for (const auto& c : clusterer->clusters)
    {
//      MESSAGE() << c.debug(true) << "\n";
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

  sorter_x->flush();
  sorter_y->flush();

  test_plane(sorter_x->clusterer, 22, 20, opts.clusterer_x.min_cluster_size);
  test_plane(sorter_y->clusterer, 0, 0, opts.clusterer_y.min_cluster_size);
}

TEST_F(DoroClustererTest, a10) {
  ReadoutFile::read(DataPath + "/readouts/a00010", readouts);
  EXPECT_EQ(readouts.size(), 920);

  add_readouts();

  sorter_x->flush();
  sorter_y->flush();

  test_plane(sorter_x->clusterer, 100, 96, opts.clusterer_x.min_cluster_size);
  test_plane(sorter_y->clusterer, 73, 68, opts.clusterer_y.min_cluster_size);
}

TEST_F(DoroClustererTest, a100) {
  ReadoutFile::read(DataPath + "/readouts/a00100", readouts);
  EXPECT_EQ(readouts.size(), 126590);

  add_readouts();

  sorter_x->flush();
  sorter_y->flush();

  test_plane(sorter_x->clusterer, 19565, 19003, opts.clusterer_x.min_cluster_size);
  test_plane(sorter_y->clusterer, 10312, 9737, opts.clusterer_y.min_cluster_size);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
