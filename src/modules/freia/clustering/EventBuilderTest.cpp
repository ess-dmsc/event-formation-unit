// Copyright (C) 2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for EventBuilder
///
//===----------------------------------------------------------------------===//

#include <multiblade/clustering/EventBuilder.h>
#include <test/TestBase.h>

using namespace Multiblade;

class EventBuilderTest : public TestBase {
protected:
  uint8_t plane_x{0};
  uint8_t plane_y{1};
  uint16_t hit_adc{4000};

  EventBuilder builder;

#include "../caen/EventBuilderCommon.cpp"
};

TEST_F(EventBuilderTest, ThresholdData) {
  ASSERT_EQ(builder.Thresholds[0][0], 34);
  ASSERT_EQ(builder.Thresholds[1][0], 33);
  ASSERT_EQ(builder.Thresholds[2][0], 137);
  ASSERT_EQ(builder.Thresholds[3][0], 142);
  ASSERT_EQ(builder.Thresholds[4][0], 143);
  ASSERT_EQ(builder.Thresholds[5][0], 31);
  ASSERT_EQ(builder.Thresholds[0][24], 8034);
}

// Preparations for Google Benchmark tests
TEST_F(EventBuilderTest, FirstTest) {
  uint32_t clusters = 10;
  uint8_t hitsperplane = 6;
  createHits(clusters, hitsperplane);

  builder.flush();

  ASSERT_EQ(builder.matcher.matched_events.size(), clusters);
  for (auto &e : builder.matcher.matched_events) {
    ASSERT_TRUE(e.both_planes());
    auto x = e.ClusterA.coord_center();
    auto y = e.ClusterB.coord_center();
    MESSAGE() << "(x,y) = (" << x << ", " << y << ")\n";
    ASSERT_FLOAT_EQ(x + 1.0, y);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
