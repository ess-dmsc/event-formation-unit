/** Copyright (C) 2019 European Spallation Source ERIC */

#include <test/TestBase.h>
#include <multiblade/clustering/EventBuilder.h>

class EventBuilderTest : public TestBase {
protected:
  uint8_t plane_x{0};
  uint8_t plane_y{1};
  uint16_t hit_adc{4000};

  Multiblade::EventBuilder builder;


  // Create Hits for numberClusters clusters, for each cluster we
  // generate 2 hits in x and 2 hits in y. All hits are separated
  // in time and space: y-coords are offset from x-coords by 1
  // just for the heck of it. Hits within a plane are separated by
  // interCoordTimeGap and hits between planes are separated by
  // interPlaneTimeGap. Finally clusters are separated by (timegap + 1)
  void createHits(uint32_t numberClusters) {
    uint64_t interCoordTimeGap = 5;
    uint64_t interPlaneTimeGap = 70; // max for this test to pass

    uint64_t t = 0;
    for (uint32_t i = 0; i < numberClusters; i++) {
      uint16_t coordStart = i % 32;
      // x-plane Hits
      builder.insert({t, coordStart    , hit_adc, plane_x});

      t += interCoordTimeGap;
      builder.insert({t, uint16_t(coordStart + 1), hit_adc, plane_x});

      // y-plane Hits
      t += interPlaneTimeGap;
      builder.insert({t, uint16_t(coordStart + 1), hit_adc, plane_y});

      t += interCoordTimeGap;
      builder.insert({t, uint16_t(coordStart + 2), hit_adc, plane_y});

      t+= timegap + 1;
    }
  }
};


// Preparations for Google Benchmark tests
TEST_F(EventBuilderTest, FirstTest) {
  uint32_t N = 10;
  createHits(N);
  
  builder.flush();

  ASSERT_EQ(builder.matcher.matched_events.size(), N);
  for (auto & e : builder.matcher.matched_events) {
    ASSERT_TRUE(e.both_planes());
    auto x = e.c1.coord_center();
    auto y = e.c2.coord_center();
    MESSAGE() << "(x,y) = (" << x << ", " << y << ")\n";
    ASSERT_FLOAT_EQ(x + 1.0 ,y);
  }

}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
