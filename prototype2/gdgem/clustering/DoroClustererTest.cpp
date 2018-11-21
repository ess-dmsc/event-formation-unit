/** Copyright (C) 2017 European Spallation Source ERIC */

#include <gdgem/nmx/Readout.h>
#include <gdgem/clustering/HitSorter.h>
#include <gdgem/clustering/DoroClusterer.h>

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <test/TestBase.h>
#include <gdgem/NMXConfig.h>
#include <functional>

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

    sorter_x->clusterer = std::make_shared<DoroClusterer>(opts.clusterer_x.max_time_gap,
                                                          opts.clusterer_x.max_strip_gap,
                                                          opts.clusterer_x.min_cluster_size);
    sorter_y->clusterer = std::make_shared<DoroClusterer>(opts.clusterer_y.max_time_gap,
                                                          opts.clusterer_y.max_strip_gap,
                                                          opts.clusterer_y.min_cluster_size);
  }

  virtual void TearDown() {
  }

  void store_hit(const Readout& readout)
  {
    uint8_t plane = opts.srs_mappings.get_plane(readout);
    EXPECT_LT(plane, 2) << "fec:" << int(readout.fec)
                        << " chip:" << int(readout.chip_id) << "\n";
    if (plane == 0) {
      sorter_x->insert(readout);
    }
    if (plane == 1) {
      sorter_y->insert(readout);
    }
  }
};

TEST_F(DoroClustererTest, a1) {
  ReadoutFile::read(DataPath + "/readouts/a00001", readouts);
  EXPECT_EQ(readouts.size(), 144);

  uint64_t bonus = 0;
  uint64_t old = 0;
  for (auto readout : readouts) {
    if (readout.srs_timestamp < old)
      bonus++;
    old = readout.srs_timestamp;
    /// \todo this hack should not be necessary!
    readout.srs_timestamp += (bonus << 42);
    store_hit(readout);
  }

  /// \todo I don't trust these results anymore, please validate
  EXPECT_EQ(sorter_x->clusterer->stats_cluster_count, 16);
  EXPECT_EQ(sorter_y->clusterer->stats_cluster_count, 0);
  sorter_x->flush();
  sorter_y->flush();
  EXPECT_EQ(sorter_x->clusterer->stats_cluster_count, 20);
  EXPECT_EQ(sorter_y->clusterer->stats_cluster_count, 0);
}

TEST_F(DoroClustererTest, a10) {
  ReadoutFile::read(DataPath + "/readouts/a00010", readouts);
  EXPECT_EQ(readouts.size(), 920);

  uint64_t bonus = 0;
  uint64_t old = 0;
  for (auto readout : readouts) {
    if (readout.srs_timestamp < old)
      bonus++;
    old = readout.srs_timestamp;
    /// \todo this hack should not be necessary!
    readout.srs_timestamp += (bonus << 42);
    store_hit(readout);
  }

  /// \todo I don't trust these results anymore, please validate
  EXPECT_EQ(sorter_x->clusterer->stats_cluster_count, 94);
  EXPECT_EQ(sorter_y->clusterer->stats_cluster_count, 66);
  sorter_x->flush();
  sorter_y->flush();
  EXPECT_EQ(sorter_x->clusterer->stats_cluster_count, 96);
  EXPECT_EQ(sorter_y->clusterer->stats_cluster_count, 68);
}

TEST_F(DoroClustererTest, a100) {
  ReadoutFile::read(DataPath + "/readouts/a00100", readouts);
  EXPECT_EQ(readouts.size(), 126590);

  uint64_t bonus = 0;
  uint64_t old = 0;
  for (auto readout : readouts) {
    if (readout.srs_timestamp < old)
      bonus++;
    old = readout.srs_timestamp;
    /// \todo this hack should not be necessary!
    readout.srs_timestamp += (bonus << 42);
    store_hit(readout);
  }

  /// \todo I don't trust these results anymore, please validate
  EXPECT_EQ(sorter_x->clusterer->stats_cluster_count, 18986);
  EXPECT_EQ(sorter_y->clusterer->stats_cluster_count, 9727);
  sorter_x->flush();
  sorter_y->flush();
  EXPECT_EQ(sorter_x->clusterer->stats_cluster_count, 18991);
  EXPECT_EQ(sorter_y->clusterer->stats_cluster_count, 9731);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
