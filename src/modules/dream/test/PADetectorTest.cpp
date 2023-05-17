
// Copyright (C) 2023 European Spallation Source ERIC, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for PADetector
///
//===----------------------------------------------------------------------===//
#include <common/testutils/TestBase.h>
#include <dream/geometry/PADetector.h>

using namespace Dream;

class PADetectorTest : public TestBase {
protected:
  DataParser::DreamReadout Data{0, 0, 0, 0, 0, 0, 0, 16, 16};

  Config::ModuleParms Parms{false, Config::ModuleType::PA, {0}, {0}};
  PADetector detb{256, 512};
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(PADetectorTest, InvalidSector) {
  Parms.P1.Sector = 7;
  ASSERT_NE(detb.getPixelId(Parms, Data), 0);
  Parms.P1.Sector = 8;
  ASSERT_EQ(detb.getPixelId(Parms, Data), 0);
}

TEST_F(PADetectorTest, ValidSector) {
  for (uint8_t Sector = 0; Sector < 8; Sector++) {
    Parms.P1.Sector = Sector;
    ASSERT_NE(detb.getPixelId(Parms, Data), 0);
  }
}

TEST_F(PADetectorTest, GetXInvalidCassette) {
  uint8_t Sector{0};
  uint8_t Counter{0};
  for (int Cassette = 0; Cassette < 16; Cassette++) {
    //                      Cassette
    ASSERT_NE(detb.getX(Sector, Cassette, Counter), -1);
  }
  ASSERT_EQ(detb.getX(Sector, 16, Counter), -1);
}

TEST_F(PADetectorTest, GetXInvalidCounter) {
  uint8_t Sector{0};
  uint8_t Cassette{1};
  //                            Sumo  Cassette  Counter
  ASSERT_NE(detb.getX(Sector, Cassette, 0), -1);
  ASSERT_NE(detb.getX(Sector, Cassette, 1), -1);
  ASSERT_EQ(detb.getX(Sector, Cassette, 2), -1);
}

TEST_F(PADetectorTest, GetYInvalidWire) {
  //                    Wire, Strip
  ASSERT_NE(detb.getY(0, 0), -1);
  ASSERT_NE(detb.getY(15, 0), -1);
  ASSERT_EQ(detb.getY(16, 0), -1);

  ASSERT_NE(detb.getY(0, 15), -1);
  ASSERT_NE(detb.getY(15, 15), -1);
  ASSERT_EQ(detb.getY(16, 15), -1);
}

TEST_F(PADetectorTest, GetYInvalidStrip) {
  //               Wire, Strip
  ASSERT_NE(detb.getY(0, 0), -1);
  ASSERT_NE(detb.getY(0, 31), -1);
  ASSERT_EQ(detb.getY(0, 32), -1);

  ASSERT_NE(detb.getY(15, 0), -1);
  ASSERT_NE(detb.getY(15, 31), -1);
  ASSERT_EQ(detb.getY(15, 32), -1);
}

TEST_F(PADetectorTest, TestingLogicalGeometryCorners) {
  //                 sec cas ctr
  ASSERT_EQ(detb.getX(0, 0, 0), 0);    // left
  ASSERT_EQ(detb.getX(7, 15, 1), 255); // right

  //                wire strip
  ASSERT_EQ(detb.getY(0, 0), 0);     // top
  ASSERT_EQ(detb.getY(15, 31), 511); // bottom
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
