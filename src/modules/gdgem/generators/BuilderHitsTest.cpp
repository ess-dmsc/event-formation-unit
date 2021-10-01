/** Copyright (C) 2019 European Spallation Source ERIC */

#include <gdgem/generators/BuilderHits.h>
#include <gdgem/srs/SRSMappings.h>
#include <common/testutils/TestBase.h>

// From common/reduction/Hit.h
std::vector<uint8_t> TwoHitsXY {
  // Hit 1 - X
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // time 2 LE
  0x01, 0x00, // coordinate 1
  0x00, 0x00, // weight 0
  0x00,       // plane 1 (y)

  // Hit 2 - Y
  0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // time 2 LE
  0x01, 0x00, // coordinate 1
  0x01, 0x00, // weight 1
  0x01,       // plane 1 (y)
};

using namespace Gem;

class BuilderHitsTest : public TestBase {
protected:
  BuilderHits BH;
  void SetUp() override {}
  void TearDown() override {}
};

// All data in BuilderHits is private
// Check counters and data structures inherited from
// AbstractBuilder
TEST_F(BuilderHitsTest, Constructor) {
  ASSERT_EQ(BH.hit_buffer_x.size(), 0);
  ASSERT_EQ(BH.hit_buffer_y.size(), 0);
}

TEST_F(BuilderHitsTest, OneHit) {
  BH.process_buffer((char*)&TwoHitsXY[0], TwoHitsXY.size());
  ASSERT_EQ(BH.hit_buffer_x.size(), 1);
  ASSERT_EQ(BH.hit_buffer_y.size(), 1);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
