
// Copyright (C) 2021 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for Dream geometry
///
//===----------------------------------------------------------------------===//
#include <common/testutils/TestBase.h>

// fails InvalidSumo test on CentOS build - false positive
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#include <dream/geometry/SUMO.h>
#pragma GCC diagnostic pop

using namespace Dream;

class DreamGeometryTest : public TestBase {
protected:
  const uint8_t Sector0{0};
  const uint8_t Sumo0{0}; // invalid. Valid range is 3 - 6
  const uint8_t Sumo3{3};
  const uint8_t Cassette0{0};
  const uint8_t Counter0{0};
  const uint8_t Wire0{0};
  const uint8_t Strip0{0};
  SUMO endcap;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(DreamGeometryTest, Constructor) {
  ASSERT_EQ(endcap.getPixel(Sector0, Sumo0, Cassette0, Counter0, Wire0, Strip0),
            0);
}

TEST_F(DreamGeometryTest, InvalidSector) {
  ASSERT_EQ(endcap.getPixel(23, Sumo3, Cassette0, Counter0, Wire0, Strip0), 0);
  ASSERT_EQ(endcap.getPixel(23, Sumo3, Cassette0, Counter0, Wire0, Strip0), 0);
  ASSERT_EQ(endcap.getPixel(24, Sumo3, Cassette0, Counter0, Wire0, Strip0), 0);
}

TEST_F(DreamGeometryTest, ValidSector) {
  for (uint8_t Sector = 0; Sector < 23; Sector++) {
    ASSERT_NE(
        endcap.getPixel(Sector, Sumo3, Cassette0, Counter0, Wire0, Strip0), 0);
  }
}

TEST_F(DreamGeometryTest, InvalidSumo) {
  ASSERT_EQ(endcap.getPixel(Sector0, 0, Cassette0, Counter0, Wire0, Strip0), 0);
  ASSERT_EQ(endcap.getPixel(Sector0, 1, Cassette0, Counter0, Wire0, Strip0), 0);
  ASSERT_EQ(endcap.getPixel(Sector0, 2, Cassette0, Counter0, Wire0, Strip0), 0);
  ASSERT_EQ(endcap.getPixel(Sector0, 7, Cassette0, Counter0, Wire0, Strip0), 0);
  ASSERT_EQ(endcap.getPixel(Sector0, 8, Cassette0, Counter0, Wire0, Strip0), 0);
}

TEST_F(DreamGeometryTest, ValidSumo) {
  ASSERT_NE(endcap.getPixel(Sector0, 3, Cassette0, Counter0, Wire0, Strip0), 0);
  ASSERT_NE(endcap.getPixel(Sector0, 4, Cassette0, Counter0, Wire0, Strip0), 0);
  ASSERT_NE(endcap.getPixel(Sector0, 5, Cassette0, Counter0, Wire0, Strip0), 0);
  ASSERT_NE(endcap.getPixel(Sector0, 6, Cassette0, Counter0, Wire0, Strip0), 0);
}

TEST_F(DreamGeometryTest, GetXInvalidCassette) {
  uint8_t Sector{0};
  uint8_t Counter{0};
  //                            Sumo  Cassette
  ASSERT_NE(endcap.getX(Sector, 3, 0, Counter), -1);
  ASSERT_EQ(endcap.getX(Sector, 3, 4, Counter), -1);
  ASSERT_NE(endcap.getX(Sector, 4, 0, Counter), -1);
  ASSERT_EQ(endcap.getX(Sector, 4, 6, Counter), -1);
  ASSERT_NE(endcap.getX(Sector, 5, 0, Counter), -1);
  ASSERT_EQ(endcap.getX(Sector, 5, 8, Counter), -1);
  ASSERT_NE(endcap.getX(Sector, 6, 0, Counter), -1);
  ASSERT_EQ(endcap.getX(Sector, 6, 10, Counter), -1);
}

TEST_F(DreamGeometryTest, GetXInvalidCounter) {
  uint8_t Sector{0};
  uint8_t Cassette{1};
  //                            Sumo  Cassette  Counter
  ASSERT_NE(endcap.getX(Sector, 3, Cassette, 0), -1);
  ASSERT_NE(endcap.getX(Sector, 3, Cassette, 1), -1);
  ASSERT_EQ(endcap.getX(Sector, 3, Cassette, 2), -1);
  ASSERT_NE(endcap.getX(Sector, 4, Cassette, 0), -1);
  ASSERT_NE(endcap.getX(Sector, 4, Cassette, 1), -1);
  ASSERT_EQ(endcap.getX(Sector, 4, Cassette, 2), -1);
  ASSERT_NE(endcap.getX(Sector, 5, Cassette, 0), -1);
  ASSERT_NE(endcap.getX(Sector, 5, Cassette, 1), -1);
  ASSERT_EQ(endcap.getX(Sector, 5, Cassette, 2), -1);
  ASSERT_NE(endcap.getX(Sector, 6, Cassette, 0), -1);
  ASSERT_NE(endcap.getX(Sector, 6, Cassette, 1), -1);
  ASSERT_EQ(endcap.getX(Sector, 6, Cassette, 2), -1);
}

TEST_F(DreamGeometryTest, GetYInvalidWire) {
  //                    Wire, Strip
  ASSERT_NE(endcap.getY(0, 0), -1);
  ASSERT_NE(endcap.getY(15, 0), -1);
  ASSERT_EQ(endcap.getY(16, 0), -1);

  ASSERT_NE(endcap.getY(0, 15), -1);
  ASSERT_NE(endcap.getY(15, 15), -1);
  ASSERT_EQ(endcap.getY(16, 15), -1);
}

TEST_F(DreamGeometryTest, GetYInvalidStrip) {
  //                    Wire, Strip
  ASSERT_NE(endcap.getY(0, 0), -1);
  ASSERT_NE(endcap.getY(0, 15), -1);
  ASSERT_EQ(endcap.getY(0, 16), -1);

  ASSERT_NE(endcap.getY(15, 0), -1);
  ASSERT_NE(endcap.getY(15, 15), -1);
  ASSERT_EQ(endcap.getY(15, 16), -1);
}

TEST_F(DreamGeometryTest, TestingICD4Corners) {
  //                        sec su cas ctr wir str
  ASSERT_EQ(endcap.getPixel(0, 6, 0, 0, 0, 0), 1);  // upper left (z = 0)
  ASSERT_EQ(endcap.getPixel(1, 6, 0, 0, 0, 0), 57); // next sector 'upper left'
  ASSERT_EQ(endcap.getPixel(22, 3, 3, 1, 0, 0), 1288);      // upper right=
  ASSERT_EQ(endcap.getPixel(0, 6, 0, 0, 15, 15), 328'441);  // bottom left
  ASSERT_EQ(endcap.getPixel(22, 3, 3, 1, 15, 15), 329'728); // bottom right
}

TEST_F(DreamGeometryTest, TestingICD4BoxCorners) {
  // Refer to figure 7 (logical geometry)
  //                    sec su cas ctr
  ASSERT_EQ(endcap.getX(0, 6, 0, 0), 0);        // top left
  ASSERT_EQ(endcap.getX(22, 3, 3, 1), 1287);    // top right
  ASSERT_EQ(endcap.getX(3, 6, 0, 0), 168 + 0);  // box left
  ASSERT_EQ(endcap.getX(3, 3, 3, 1), 168 + 55); // box right

  //                    wir str
  ASSERT_EQ(endcap.getY(0, 3), 48 + 0);   // box top
  ASSERT_EQ(endcap.getY(15, 3), 48 + 15); // box bottom
  ASSERT_EQ(endcap.getY(0, 0), 0);        // top
  ASSERT_EQ(endcap.getY(15, 15), 255);    // bottom
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
