/** Copyright (C) 2019 European Spallation Source ERIC */

#include <gdgem/generators/BuilderReadouts.h>
#include <gdgem/srs/SRSMappings.h>
#include <test/TestBase.h>

std::vector<uint8_t> OneReadout {
  0x00, // fec 0
  0x00, // chip 0
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // time 2 LE
  0x01, 0x00, // channel 1
  0xbb, 0xaa, // bcid 0xaabb
  0x01, 0x00, // tdc 0x01
  0x23, 0x01, // adc 0x0123
  0x00,       // over threshold (bool?) false
  0x00, 0x00, 0x00, 0x00, // chiptime (float?) 0.0
};

std::vector<uint8_t> OneReadoutY {
  0x01, // fec 0
  0x00, // chip 0
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // time 2 LE
  0x01, 0x00, // channel 1
  0xbb, 0xaa, // bcid 0xaabb
  0x01, 0x00, // tdc 0x01
  0x23, 0x01, // adc 0x0123
  0x00,       // over threshold (bool?) false
  0x00, 0x00, 0x00, 0x00, // chiptime (float?) 0.0
};

std::vector<uint8_t> OneReadoutBadCoord {
  0x00, // fec 0
  0x00, // chip 0
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // time 2 LE
  0xff, 0xff, // channel 0xffff
  0xbb, 0xaa, // bcid 0xaabb
  0x01, 0x00, // tdc 0x01
  0x23, 0x01, // adc 0x0123
  0x00,       // over threshold (bool?) false
  0x00, 0x00, 0x00, 0x00, // chiptime (float?) 0.0
};

std::vector<uint8_t> OneReadoutAdcUnderThreshold {
  0x00, // fec 0
  0x00, // chip 0
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // time 2 LE
  0x01, 0x00, // channel 1
  0xbb, 0xaa, // bcid 0xaabb
  0x01, 0x00, // tdc 0x01
  0x01, 0x00, // adc 0x0001
  0x00,       // over threshold (bool?) false
  0x00, 0x00, 0x00, 0x00, // chiptime (float?) 0.0
};

std::vector<uint8_t> OneReadoutBadPlane {
  0x10, // fec 0
  0x10, // chip 0
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // time 2 LE
  0x01, 0x00, // channel 1
  0xbb, 0xaa, // bcid 0xaabb
  0x01, 0x00, // tdc 0x01
  0x23, 0x01, // adc 0x0123
  0x00,       // over threshold (bool?) false
  0x00, 0x00, 0x00, 0x00, // chiptime (float?) 0.0
};

using namespace Gem;

class BuilderReadoutsTest : public TestBase {
protected:
  SRSMappings Mappings;
  const uint16_t AdcThreshold{128};
  const std::string DumpDirectory{"./"};

  void SetUp() override {
    // Define minimal mappings (copied from SRSMappingstest)
    // Fec 0, VMM 0, Plane 0, channel offset 0
    Mappings.set_mapping(0, 0, 0, 0);

    // Fec 0, VMM 0, Plane 0, channel offset 0
    Mappings.set_mapping(1, 0, 1, 0);
  }
  void TearDown() override {}
};

// Most data in BuilderReadouts is private
// Check counters and data structures inherited from
// AbstractBuilder
TEST_F(BuilderReadoutsTest, Constructor) {
  BuilderReadouts BR(Mappings, AdcThreshold, DumpDirectory);
  ASSERT_EQ(BR.stats.ParserReadouts, 0);
  ASSERT_EQ(BR.stats.geom_errors, 0);
  ASSERT_EQ(BR.hit_buffer_x.size(), 0);
  ASSERT_EQ(BR.hit_buffer_y.size(), 0);
}

TEST_F(BuilderReadoutsTest, BadPlane) {
  BuilderReadouts BR(Mappings, AdcThreshold, DumpDirectory);
  BR.process_buffer((char*)&OneReadoutBadPlane[0], OneReadoutBadPlane.size());
  ASSERT_EQ(BR.stats.geom_errors, 1);
}

TEST_F(BuilderReadoutsTest, BadCoord) {
  BuilderReadouts BR(Mappings, AdcThreshold, DumpDirectory);
  BR.process_buffer((char*)&OneReadoutBadCoord[0], OneReadoutBadCoord.size());
  ASSERT_EQ(BR.stats.geom_errors, 1);
}

TEST_F(BuilderReadoutsTest, ParseOneReadoutX) {
  BuilderReadouts BR(Mappings, AdcThreshold, DumpDirectory);
  BR.process_buffer((char*)&OneReadout[0], OneReadout.size());
  ASSERT_EQ(BR.stats.geom_errors, 0);
  ASSERT_EQ(BR.hit_buffer_x.size(), 1);
  ASSERT_EQ(BR.hit_buffer_y.size(), 0);
}

TEST_F(BuilderReadoutsTest, ParseOneReadoutY) {
  BuilderReadouts BR(Mappings, AdcThreshold, DumpDirectory);
  BR.process_buffer((char*)&OneReadoutY[0], OneReadoutY.size());
  ASSERT_EQ(BR.stats.geom_errors, 0);
  ASSERT_EQ(BR.hit_buffer_x.size(), 0);
  ASSERT_EQ(BR.hit_buffer_y.size(), 1);
}

TEST_F(BuilderReadoutsTest, AdcUnderThreshold) {
  BuilderReadouts BR(Mappings, AdcThreshold, DumpDirectory);
  BR.process_buffer((char*)&OneReadoutAdcUnderThreshold[0],
        OneReadoutAdcUnderThreshold.size());
  ASSERT_EQ(BR.stats.geom_errors, 0);
  ASSERT_EQ(BR.stats.adc_rejects, 1);
  ASSERT_EQ(BR.hit_buffer_x.size(), 0);
  ASSERT_EQ(BR.hit_buffer_y.size(), 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
