/** Copyright (C) 2017 European Spallation Source ERIC */

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <gdgem/clustering/HitSorter.h>
#include <test/TestBase.h>
#include <functional>

#include <gdgem/clustering/TestDataShort.h>
#include <gdgem/nmx/ReadoutFile.h>

#define UNUSED __attribute__((unused))

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
  std::vector<Readout> long_data;
  std::vector<Readout> super_long_data;

  uint16_t pADCThreshold = 0;
  double pMaxTimeGap = 200;

  SRSMappings mapping;

  std::shared_ptr<MockClusterer> mock_x;
  std::shared_ptr<MockClusterer> mock_y;
  std::shared_ptr<HitSorter> sorter_x;
  std::shared_ptr<HitSorter> sorter_y;

  virtual void SetUp() {
    std::string DataPath = TEST_DATA_PATH;
    ReadoutFile::read(DataPath + "run16long.h5", long_data);
    ReadoutFile::read(DataPath + "run16full.h5", super_long_data);

    mapping.set_mapping(1, 0, 0, 0);
    mapping.set_mapping(1, 1, 0, 64);
    mapping.set_mapping(1, 6, 0, 128);
    mapping.set_mapping(1, 7, 0, 192);

    mapping.set_mapping(1, 10, 1, 0);
    mapping.set_mapping(1, 11, 1, 64);
    mapping.set_mapping(1, 14, 1, 128);
    mapping.set_mapping(1, 15, 1, 192);

    SRSTime srstime;
    srstime.set_bc_clock(20);
    srstime.set_tac_slope(60);
    srstime.set_trigger_resolution_ns(3.125);
    srstime.set_acquisition_window(4000);

    mock_x = std::make_shared<MockClusterer>();
    mock_y = std::make_shared<MockClusterer>();
    sorter_x = std::make_shared<HitSorter>(srstime, mapping, pADCThreshold, pMaxTimeGap);
    sorter_y = std::make_shared<HitSorter>(srstime, mapping, pADCThreshold, pMaxTimeGap);
    sorter_x->clusterer = mock_x;
    sorter_y->clusterer = mock_y;
  }

  virtual void TearDown() {
  }

  void store_hit(const Readout& readout)
  {
    uint8_t planeID = mapping.get_plane(readout);
    if (planeID == 1) {
      sorter_y->insert(readout);
    } else {
      sorter_x->insert(readout);
    }
  }
};

TEST_F(HitSorterTest, BcidTdcError) {
  EXPECT_EQ(0, sorter_x->stats_bcid_tdc_error);
  EXPECT_EQ(0, sorter_y->stats_bcid_tdc_error);

  for (const auto& readout : err_bcid_tdc_error) {
    store_hit(readout);
  }
  // Two in X and Two in Y
  EXPECT_EQ(2, sorter_x->stats_bcid_tdc_error);
  EXPECT_EQ(2, sorter_y->stats_bcid_tdc_error);
  EXPECT_EQ(1, sorter_x->stats_trigger_count);
  EXPECT_EQ(1, sorter_y->stats_trigger_count);
}

TEST_F(HitSorterTest, FrameCounterError) {
  EXPECT_EQ(0, sorter_x->stats_fc_error);
  EXPECT_EQ(0, sorter_y->stats_fc_error);

  for (const auto& readout : err_fc_error) {
    store_hit(readout);
  }
  EXPECT_EQ(1, sorter_x->stats_fc_error);
  EXPECT_EQ(0, sorter_y->stats_fc_error);
  EXPECT_EQ(1, sorter_x->stats_trigger_count);
  EXPECT_EQ(0, sorter_y->stats_trigger_count);
}

TEST_F(HitSorterTest, TriggerTimeWraps) {
  EXPECT_EQ(0, sorter_x->stats_triggertime_wraps);
  EXPECT_EQ(0, sorter_y->stats_triggertime_wraps);

  for (const auto& readout : err_triggertime_error) {
    store_hit(readout);
  }
  EXPECT_EQ(0, sorter_x->stats_triggertime_wraps);
  EXPECT_EQ(1, sorter_y->stats_triggertime_wraps);

  EXPECT_EQ(0, sorter_x->stats_trigger_count);
  EXPECT_EQ(2, sorter_y->stats_trigger_count);
}

TEST_F(HitSorterTest, Run16_Short) {
  for (const auto& readout : Run16) {
    store_hit(readout);
  }
  EXPECT_EQ(0, sorter_x->stats_triggertime_wraps);
  EXPECT_EQ(0, sorter_y->stats_triggertime_wraps);
  EXPECT_EQ(0, sorter_x->stats_bcid_tdc_error);
  EXPECT_EQ(0, sorter_y->stats_bcid_tdc_error);
  EXPECT_EQ(0, sorter_x->stats_fc_error);
  EXPECT_EQ(0, sorter_y->stats_fc_error);

  EXPECT_EQ(2, sorter_x->stats_trigger_count);
  EXPECT_EQ(2, sorter_y->stats_trigger_count);
}

TEST_F(HitSorterTest, Run16_Long) {
  for (const auto& readout : long_data) {
    store_hit(readout);
  }
  EXPECT_EQ(0, sorter_x->stats_triggertime_wraps);
  EXPECT_EQ(0, sorter_y->stats_triggertime_wraps);
  EXPECT_EQ(0, sorter_x->stats_bcid_tdc_error);
  EXPECT_EQ(0, sorter_y->stats_bcid_tdc_error);
  EXPECT_EQ(0, sorter_x->stats_fc_error);
  EXPECT_EQ(0, sorter_y->stats_fc_error);

  // Need an intermediate-size dataset where this can be confirmed analytically
  EXPECT_EQ(1539, sorter_x->stats_trigger_count);
  EXPECT_EQ(1540, sorter_y->stats_trigger_count);
}

TEST_F(HitSorterTest, Mock_short_chrono) {
  for (const auto& readout : Run16) {
    store_hit(readout);
  }

  EXPECT_EQ(mock_x->stats_chrono_errors, 0);
  EXPECT_EQ(mock_y->stats_chrono_errors, 0);

  // flush, but must it be with trigger?
  sorter_x->flush();
  sorter_y->flush();

  EXPECT_EQ(55, mock_x->all_hits.size());
  EXPECT_EQ(101, mock_y->all_hits.size());
  EXPECT_EQ(Run16.size(), (mock_x->all_hits.size() + mock_y->all_hits.size()));

  //TODO: why is this failing?
//  EXPECT_EQ(mock_x->stats_chrono_errors, 0);
//  EXPECT_EQ(mock_y->stats_chrono_errors, 0);
}

TEST_F(HitSorterTest, Mock_long_chrono) {
  for (const auto& readout : long_data) {
    store_hit(readout);
  }

  //TODO: why is this failing?

//  EXPECT_EQ(mock_x->stats_chrono_errors, 0);
//  EXPECT_EQ(mock_y->stats_chrono_errors, 0);

  // flush, but must it be with trigger?
  sorter_x->flush();
  sorter_y->flush();

  EXPECT_EQ(84168, mock_x->all_hits.size());
  EXPECT_EQ(115827, mock_y->all_hits.size());
  EXPECT_EQ(long_data.size(), (mock_x->all_hits.size() + mock_y->all_hits.size()));
}

TEST_F(HitSorterTest, Mock_super_long_chrono) {
  for (const auto& readout : super_long_data) {
    store_hit(readout);
  }

  //TODO: why is this failing?

//  EXPECT_EQ(mock_x->stats_chrono_errors, 0);
//  EXPECT_EQ(mock_y->stats_chrono_errors, 0);

  // flush, but must it be with trigger?
  sorter_x->flush();
  sorter_y->flush();

  EXPECT_EQ(441260, mock_x->all_hits.size());
  EXPECT_EQ(610127, mock_y->all_hits.size());
  EXPECT_EQ(super_long_data.size(), (mock_x->all_hits.size() + mock_y->all_hits.size()));
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
