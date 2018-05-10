/** Copyright (C) 2017 European Spallation Source ERIC */

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <gdgem/dg_impl/HitSorter.h>
#include <test/TestBase.h>
#include <functional>

#include <gdgem/dg_impl/TestDataShort.h>
#include <gdgem/dg_impl/TestDataLong.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

#define UNUSED __attribute__((unused))

class NMXClustererTest : public TestBase {
protected:
  uint16_t pADCThreshold = 0;
  double pMaxTimeGap = 200;

  SRSMappings mapping;

  std::shared_ptr<HitSorter> sorter_x;
  std::shared_ptr<HitSorter> sorter_y;

  virtual void SetUp() {
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
    srstime.set_trigger_resolution(3.125);
    srstime.set_acquisition_window(4000);

    sorter_x = std::make_shared<HitSorter>(srstime, mapping, pADCThreshold, pMaxTimeGap, nullptr);
    sorter_y = std::make_shared<HitSorter>(srstime, mapping, pADCThreshold, pMaxTimeGap, nullptr);
  }

  virtual void TearDown() {
  }
};

TEST_F(NMXClustererTest, BcidTdcError) {
  EXPECT_EQ(0, sorter_x->stats_bcid_tdc_error);
  EXPECT_EQ(0, sorter_y->stats_bcid_tdc_error);

  for (auto hit : err_bcid_tdc_error) {
    uint8_t planeID = mapping.get_plane(hit.fec, hit.chip_id);
    if (planeID == 1) {
      sorter_y->store(hit.srs_timestamp, hit.framecounter,
                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                      hit.adc,
                      hit.overthreshold);
    } else {
      sorter_x->store(hit.srs_timestamp, hit.framecounter,
                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                      hit.adc,
                      hit.overthreshold);
    }
  }
  // Two in X and Two in Y
  EXPECT_EQ(2, sorter_x->stats_bcid_tdc_error);
  EXPECT_EQ(2, sorter_y->stats_bcid_tdc_error);
  EXPECT_EQ(1, sorter_x->stats_trigger_count);
  EXPECT_EQ(1, sorter_y->stats_trigger_count);
}

TEST_F(NMXClustererTest, FrameCounterError) {
  EXPECT_EQ(0, sorter_x->stats_fc_error);
  EXPECT_EQ(0, sorter_y->stats_fc_error);

  for (auto hit : err_fc_error) {
    uint8_t planeID = mapping.get_plane(hit.fec, hit.chip_id);
    if (planeID == 1) {
      sorter_y->store(hit.srs_timestamp, hit.framecounter,
                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                      hit.adc,
                      hit.overthreshold);
    } else {
      sorter_x->store(hit.srs_timestamp, hit.framecounter,
                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                      hit.adc,
                      hit.overthreshold);
    }
  }
  EXPECT_EQ(1, sorter_x->stats_fc_error);
  EXPECT_EQ(0, sorter_y->stats_fc_error);
  EXPECT_EQ(1, sorter_x->stats_trigger_count);
  EXPECT_EQ(0, sorter_y->stats_trigger_count);
}

TEST_F(NMXClustererTest, TriggerTimeWraps) {
  EXPECT_EQ(0, sorter_x->stats_triggertime_wraps);
  EXPECT_EQ(0, sorter_y->stats_triggertime_wraps);

  for (auto hit : err_triggertime_error) {
    uint8_t planeID = mapping.get_plane(hit.fec, hit.chip_id);
    if (planeID == 1) {
      sorter_y->store(hit.srs_timestamp, hit.framecounter,
                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                      hit.adc,
                      hit.overthreshold);
    } else {
      sorter_x->store(hit.srs_timestamp, hit.framecounter,
                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                      hit.adc,
                      hit.overthreshold);
    }
  }
  EXPECT_EQ(0, sorter_x->stats_triggertime_wraps);
  EXPECT_EQ(1, sorter_y->stats_triggertime_wraps);

  EXPECT_EQ(0, sorter_x->stats_trigger_count);
  EXPECT_EQ(2, sorter_y->stats_trigger_count);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST_F(NMXClustererTest, Run16_line_110168_110323) {
  for (auto hit : Run16) {
    uint8_t planeID = mapping.get_plane(hit.fec, hit.chip_id);
    if (planeID == 1) {
      sorter_y->store(hit.srs_timestamp, hit.framecounter,
                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                      hit.adc,
                      hit.overthreshold);
    } else {
      sorter_x->store(hit.srs_timestamp, hit.framecounter,
                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                      hit.adc,
                      hit.overthreshold);
    }
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

TEST_F(NMXClustererTest, Run16_Long) {
  for (auto hit : Run16_Long) {
    uint8_t planeID = mapping.get_plane(hit.fec, hit.chip_id);
    if (planeID == 1) {
      sorter_y->store(hit.srs_timestamp, hit.framecounter,
                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                      hit.adc,
                      hit.overthreshold);
    } else {
      sorter_x->store(hit.srs_timestamp, hit.framecounter,
                      hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc,
                      hit.adc,
                      hit.overthreshold);
    }
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

// Make some tests using a mock clusterer