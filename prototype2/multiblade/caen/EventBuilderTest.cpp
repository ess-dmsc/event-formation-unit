/** Copyright (C) 2019 European Spallation Source ERIC */

#include <test/TestBase.h>
#include <multiblade/clustering/EventBuilder.h>

class EventBuilderTest : public TestBase {
protected:
  uint8_t plane_x{0};
  uint8_t plane_y{1};
  uint16_t hit_adc{4000};

  Multiblade::EventBuilder builder;

  #include "EventBuilderCommon.cpp"
};


// Preparations for Google Benchmark tests
TEST_F(EventBuilderTest, FirstTest) {
  uint32_t clusters = 10;
  uint8_t hitsperplane = 6;
  createHits(clusters, hitsperplane);

  builder.flush();

  ASSERT_EQ(builder.matcher.matched_events.size(), clusters);
  for (auto & e : builder.matcher.matched_events) {
    ASSERT_TRUE(e.both_planes());
    auto x = e.cluster1.coord_center();
    auto y = e.cluster2.coord_center();
    MESSAGE() << "(x,y) = (" << x << ", " << y << ")\n";
    ASSERT_FLOAT_EQ(x + 1.0 ,y);
  }

}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
