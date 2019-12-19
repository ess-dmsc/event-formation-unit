/** Copyright (C) 2017 European Spallation Source ERIC */

#include <test/TestBase.h>
#include <gdgem/tests/HitGenerator.h>
#include <common/reduction/Hit.h>
#include <common/reduction/clustering/GapClusterer.h>
#include <cmath>

using namespace Gem;

class HitGeneratorTest : public TestBase {
protected:
  float D2R(float Degree) {
    return 2 * M_PI * Degree / 360.0;
  }
  void SetUp() override { }

  void TearDown() override { }
};

TEST_F(HitGeneratorTest, Constructor) {
  HitGenerator HitGen;
}

TEST_F(HitGeneratorTest, GenerateAngle0) {
  HitGenerator HitGen;
  float Angle0{0.0};
  uint16_t NumHits{6};
  HitGen.setTimes(200, 40, 1);
  auto hits = HitGen.makeHit(NumHits, 0, 0, Angle0, false);
  ASSERT_TRUE(hits.size() <= NumHits*2);
  HitGen.printHits();
}

TEST_F(HitGeneratorTest, GenerateAngle45) {
  HitGenerator HitGen;
  float Angle45 = D2R(45);
  uint16_t NumHits{6};
  HitGen.setTimes(200, 40, 1);
  auto hits = HitGen.makeHit(NumHits, 0, 0, Angle45, false);
  ASSERT_TRUE(hits.size() <= NumHits*2);
  HitGen.printHits();
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
