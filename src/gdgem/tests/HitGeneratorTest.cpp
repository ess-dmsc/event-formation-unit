/** Copyright (C) 2017 European Spallation Source ERIC */

#include <test/TestBase.h>
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

  uint8_t Gap0{0};
  uint32_t DeadTime0{0};
  uint32_t DeadTime200{200};
  bool Shuffle{true};
  bool NoShuffle{false};

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
TEST_F(HitGeneratorTest, Generate10Angle90Offset100,DT0) {
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
  HitGen.printHits();
  ASSERT_EQ(H.size(), 1);
  for (auto & hit : H ) {
    ASSERT_TRUE(hit.time >= 200);
    ASSERT_TRUE(hit.coordinate == 100);
  }
}

TEST_F(HitGeneratorTest, Generate90Offset100DT1) {
  HitGenerator HitGen;
  HitGen.setTimeParms(200, 40, 1);
  auto & H = HitGen.makeHitsForSinglePlane(0, 10, 100, 100, Angle90, Gap0, 2, NoShuffle);
  HitGen.printHits();
  ASSERT_EQ(H.size(), 5); // Every second should be accepted
  for (auto & hit : H ) {
    ASSERT_TRUE(hit.time >= 200);
    ASSERT_TRUE(hit.coordinate == 100);
  }
}


// TEST_F(HitGeneratorTest, GenerateAnglesPlaneX) {
//   uint16_t MaxHits{6};
//   for (uint16_t xoffset = 0; xoffset < 10; xoffset++) {
//     {
//       printf("Test 1 - 0 Degree\n");
//       HitGenerator HitGen;
//       HitGen.setTimeParms(200, 40, 1);
//       auto Generated = HitGen.makeHitsForSinglePlane(0, MaxHits, xoffset, 0, Angle0, Gap0, 200, NoShuffle);
//       ASSERT_EQ(Generated.size(), MaxHits);
//       HitGen.printHits();
//     }
//
//     {
//       printf("Test 1 - 90 Degree\n");
//       HitGenerator HitGen;
//       HitGen.setTimeParms(200, 40, 1);
//       auto Generated = HitGen.makeHitsForSinglePlane(0, MaxHits, xoffset, 0, Angle90, Gap0, 200, NoShuffle);
//       HitGen.printHits();
//       ASSERT_EQ(Generated.size(), 1);
//
//     }
//
//     {
//       printf("Test 1 - 180 Degree\n");
//       HitGenerator HitGen;
//       HitGen.setTimeParms(200, 40, 1);
//       auto Generated = HitGen.makeHitsForSinglePlane(0, MaxHits, xoffset, 0, Angle180, Gap0, 200, NoShuffle);
//       ASSERT_EQ(Generated.size(), std::min(uint16_t(xoffset + 1), MaxHits));
//       HitGen.printHits();
//     }
//
//     {
//       printf("Test 1 - 270 Degree\n");
//       HitGenerator HitGen;
//       HitGen.setTimeParms(200, 40, 1);
//       auto Generated = HitGen.makeHitsForSinglePlane(0, MaxHits, xoffset, 0, Angle270, Gap0, 200, NoShuffle);
//       ASSERT_EQ(Generated.size(), 1);
//       HitGen.printHits();
//     }
//   }
// }

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
