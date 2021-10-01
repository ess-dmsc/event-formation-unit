/** Copyright (C) 2017 European Spallation Source ERIC */

#include <common/testutils/TestBase.h>
#include <gdgem/tests/HitGenerator.h>
#include <common/reduction/Hit.h>
#include <common/reduction/clustering/GapClusterer.h>
#include <cmath>

using namespace Gem;

class HitGeneratorTest : public TestBase {
protected:
  HitGenerator HitGen;
  float Angle0{0.0};
  float Angle45{45.0};
  float Angle90{90.0};
  float Angle180{180.0};
  float Angle270{270.0};

  uint8_t Gap0{0}, Gap1{1}, Gap2{2};
  uint32_t DeadTime0{0}, DeadTime200{200};
  bool Shuffle{true}, NoShuffle{false};

  bool validate(uint16_t x, uint16_t y, float angle, size_t nx, size_t ny) {
    HitGenerator GenX, GenY;
    auto & Hx = GenX.makeHitsForSinglePlane(0, 10, x, y, angle, Gap0, DeadTime0, NoShuffle);
    auto & Hy = GenY.makeHitsForSinglePlane(1, 10, x, y, angle, Gap0, DeadTime0, NoShuffle);
    return ((Hx.size() == nx) and (Hy.size() == ny));
  }

  void SetUp() override {
    HitGen.setTimeParms(200, 40, 1);
  }
  void TearDown() override { }
};

TEST_F(HitGeneratorTest, Constructor) {
  auto & H = HitGen.getHits();
  ASSERT_EQ(H.size(), 0);
}

TEST_F(HitGeneratorTest, Generate10Angle0Offset100DT0) {
  auto & H = HitGen.makeHitsForSinglePlane(0, 10, 100, 100, Angle0, Gap0, DeadTime0, NoShuffle);
  ASSERT_EQ(H.size(), 10);
  uint16_t coord = 100;
  for (auto & hit : H ) {
    ASSERT_TRUE(hit.time >= 200);
    ASSERT_TRUE(hit.coordinate == coord++);
  }
}

// With dead time 0ns all Hits should be accepted
TEST_F(HitGeneratorTest, Generate10Angle90Offset100DT0) {
  auto & H = HitGen.makeHitsForSinglePlane(0, 10, 100, 100, Angle90, Gap0, DeadTime0, NoShuffle);
  ASSERT_EQ(H.size(), 10); // Unrealistic dead time, should allow all Hits
  for (auto & hit : H ) {
    ASSERT_TRUE(hit.time >= 200);
    ASSERT_TRUE(hit.coordinate == 100);
  }
}

// Dead time 200 ns much larger than 1 ns between Hits, all but first
// should be discarded
TEST_F(HitGeneratorTest, Generate10Angle90Offset100DT200) {
  auto & H = HitGen.makeHitsForSinglePlane(0, 10, 100, 100, Angle90, Gap0, DeadTime200, NoShuffle);
  ASSERT_EQ(H.size(), 1);
  for (auto & hit : H ) {
    ASSERT_TRUE(hit.time >= 200);
    ASSERT_TRUE(hit.coordinate == 100);
  }
}

TEST_F(HitGeneratorTest, DeadTime2) {
  auto & H = HitGen.makeHitsForSinglePlane(0, 10, 100, 100, Angle90, Gap0, 2, NoShuffle);
  ASSERT_EQ(H.size(), 5); // Every second should be accepted
  for (auto & hit : H ) {
    ASSERT_TRUE(hit.time >= 200);
    ASSERT_TRUE(hit.coordinate == 100);
  }
}

TEST_F(HitGeneratorTest, Borders) {
  // Run through all borders in four directions, check that regions are
  // obeyed
  for (uint16_t i = 0; i < 1280; i++) {
    // Left border
    ASSERT_TRUE(validate(0, i, Angle0,   10, 10));
    ASSERT_TRUE(validate(0, i, Angle90,  10, std::min(10U, 1280U - i)));
    ASSERT_TRUE(validate(0, i, Angle180,  1, 10));
    ASSERT_TRUE(validate(0, i, Angle270, 10, std::min(10U,    1U + i)));

    // Right border
    ASSERT_TRUE(validate(1279, i, Angle0,    1, 10));
    ASSERT_TRUE(validate(1279, i, Angle90,  10, std::min(10U, 1280U - i)));
    ASSERT_TRUE(validate(1279, i, Angle180, 10, 10));
    ASSERT_TRUE(validate(1279, i, Angle270, 10, std::min(10U,    1U + i)));

    // Bottom border
    ASSERT_TRUE(validate(i, 0, Angle0,   std::min(10U, 1280U - i), 10));
    ASSERT_TRUE(validate(i, 0, Angle90,  10, 10));
    ASSERT_TRUE(validate(i, 0, Angle180, std::min(10U, 1U + i), 10));
    ASSERT_TRUE(validate(i, 0, Angle270, 10,  1));

    // Top border
    ASSERT_TRUE(validate(i, 1279, Angle0,   std::min(10U, 1280U - i), 10));
    ASSERT_TRUE(validate(i, 1279, Angle90,  10,  1));
    ASSERT_TRUE(validate(i, 1279, Angle180, std::min(10U, 1U + i), 10));
    ASSERT_TRUE(validate(i, 1279, Angle270, 10, 10));
  }
}


TEST_F(HitGeneratorTest, InvalidPlane) {
  for (int i = 2; i < 256; i++) {
    auto & Hits = HitGen.makeHitsForSinglePlane(i, 100, 640, 640, Angle45, Gap0, 2, NoShuffle);
    ASSERT_EQ(Hits.size(), 0);
  }
}


TEST_F(HitGeneratorTest, NoShuffle) {
  // Expect channels and times to be strictly increasing
  auto & Hits = HitGen.makeHitsForSinglePlane(0, 100, 640, 640, Angle45, Gap0, 2, NoShuffle);
  uint32_t Time{0}, Coordinate{0};
  for (auto & H : Hits) {
    ASSERT_TRUE(H.time > Time);
    Time = H.time;
    ASSERT_TRUE(H.coordinate > Coordinate);
    Coordinate = H.coordinate;
  }
}

TEST_F(HitGeneratorTest, Shuffle) {
  // Expect channels and times to be strictly increasing
  auto & Hits = HitGen.makeHitsForSinglePlane(0, 100, 640, 640, Angle45, Gap0, 2, Shuffle);
  uint32_t Time{0}, Coordinate{0}, OutOfOrder{0};
  for (auto & H : Hits) {
    if (H.time < Time) {
      OutOfOrder++;
    }
    Time = H.time;
    if (H.coordinate < Coordinate) {
      OutOfOrder++;
    }
    Coordinate = H.coordinate;
  }
  ASSERT_TRUE(OutOfOrder != 0);
}


TEST_F(HitGeneratorTest, Gaps) {
  for (unsigned int Gaps = 0; Gaps < 9; Gaps++) {
    HitGenerator GapGen;
    auto & Hits = GapGen.makeHitsForSinglePlane(0, 10, 640, 640, Angle0, Gaps, DeadTime200, NoShuffle);
    ASSERT_EQ(Hits.size(), 10 - Gaps);
  }
  for (unsigned int Gaps = 9; Gaps < 100; Gaps++) {
    HitGenerator GapGen;
    auto & Hits = GapGen.makeHitsForSinglePlane(0, 10, 640, 640, Angle0, Gaps, DeadTime200, NoShuffle);
    ASSERT_EQ(Hits.size(), 0);
  }
}

TEST_F(HitGeneratorTest, RandomEvents) {
  HitGenerator RandGen;
  int NBEvents{10000}, MaxHits{6};

  // First generate NeutronEvents
  auto & Events = RandGen.randomEvents(NBEvents, 20, 1259);
  ASSERT_EQ(Events.size(), NBEvents);
  for (auto & Event : Events) {
    ASSERT_TRUE(Event.XPos >= 10 and Event.XPos <= 1259);
    ASSERT_TRUE(Event.YPos >= 10 and Event.YPos <= 1259);
  }
  //RandGen.printEvents(); // You might want to reduce NBEvents

  // Then generate hits from the neutron events
  auto & Hits = RandGen.randomHits(MaxHits, Gap0, DeadTime200, NoShuffle);
  ASSERT_TRUE((int)Hits.size() > NBEvents * 2);
  ASSERT_TRUE((int)Hits.size() <= NBEvents * 2 * MaxHits);
  //RandGen.printHits(); // You might want to reduce NBEvents
}



// NOT A TEST just printing
TEST_F(HitGeneratorTest, PrintEventsAndHits) {
  HitGen.randomEvents(10, 0, 1279);
  HitGen.printEvents();
  HitGen.randomHits(12, Gap0, DeadTime200, NoShuffle);
  HitGen.printHits();
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
