
// Copyright (C) 2021 - 2024 European Spallation Source, see LICENSE file
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
  DataParser::CDTReadout Data{0, 0, 0, 0, 0, 0, 0, 16, 16};
  Config::ModuleParms Parms{false, Config::ModuleType::BwEndCap, {0}, {0}};
  SUMO endcap{616, 256};
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(DreamGeometryTest, Constructor) {
  ASSERT_EQ(endcap.getPixelId(Parms, Data), 0);
}

TEST_F(DreamGeometryTest, InvalidSector) {
  Parms.P1.Sector = 23;
  ASSERT_EQ(endcap.getPixelId(Parms, Data), 0);
  Parms.P1.Sector = 24;
  ASSERT_EQ(endcap.getPixelId(Parms, Data), 0);
}

TEST_F(DreamGeometryTest, ValidSector) {
  Data.UnitId = 6;
  for (uint8_t Sector = 0; Sector < 11; Sector++) {
    Parms.P1.Sector = Sector;
    ASSERT_NE(endcap.getPixelId(Parms, Data), 0);
  }
}

TEST_F(DreamGeometryTest, InvalidSumo) {
  std::vector<int> SumoIDs{0, 1, 2, 7, 8};
  for (auto const &ID : SumoIDs) {
    Data.UnitId = ID;
    ASSERT_EQ(endcap.getPixelId(Parms, Data), 0);
  }
}

TEST_F(DreamGeometryTest, ValidSumo) {
  std::vector<int> SumoIDs{3, 4, 5, 6};
  for (auto const &ID : SumoIDs) {
    Data.UnitId = ID;
    ASSERT_NE(endcap.getPixelId(Parms, Data), 0);
  }
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

// Testing CDT specified mappings
// From dream_voxel_position_relations[3].png
// provided by Daniel Hollain 30 Jan 2023
TEST_F(DreamGeometryTest, CDTMappingSumo6) {
  ASSERT_EQ(endcap.getCassette(6, 0, 0), 0);
  ASSERT_EQ(endcap.getCassette(6, 31, 15), 0);

  ASSERT_EQ(endcap.getCassette(6, 0, 16), 2);
  ASSERT_EQ(endcap.getCassette(6, 32, 16), 1);
  ASSERT_EQ(endcap.getCassette(6, 0, 32), 4);
  ASSERT_EQ(endcap.getCassette(6, 32, 32), 3);
  ASSERT_EQ(endcap.getCassette(6, 0, 48), 6);
  ASSERT_EQ(endcap.getCassette(6, 32, 48), 5);
  ASSERT_EQ(endcap.getCassette(6, 0, 64), 8);
  ASSERT_EQ(endcap.getCassette(6, 32, 64), 7);
  ASSERT_EQ(endcap.getCassette(6, 32, 80), 9);
}

TEST_F(DreamGeometryTest, CDTMappingSumo5) {
  ASSERT_EQ(endcap.getCassette(5, 0, 0), 0);
  ASSERT_EQ(endcap.getCassette(5, 32, 0), 1);
  ASSERT_EQ(endcap.getCassette(5, 0, 16), 2);
  ASSERT_EQ(endcap.getCassette(5, 32, 16), 3);
  ASSERT_EQ(endcap.getCassette(5, 0, 32), 4);
  ASSERT_EQ(endcap.getCassette(5, 32, 32), 5);
  ASSERT_EQ(endcap.getCassette(5, 0, 48), 6);
  ASSERT_EQ(endcap.getCassette(5, 32, 48), 7);
}

TEST_F(DreamGeometryTest, CDTMappingSumo4) {
  ASSERT_EQ(endcap.getCassette(4, 32, 0), 0);
  ASSERT_EQ(endcap.getCassette(4, 0, 16), 1);
  ASSERT_EQ(endcap.getCassette(4, 32, 16), 2);
  ASSERT_EQ(endcap.getCassette(4, 0, 32), 3);
  ASSERT_EQ(endcap.getCassette(4, 32, 32), 4);
  ASSERT_EQ(endcap.getCassette(4, 0, 48), 5);
}

TEST_F(DreamGeometryTest, CDTMappingSumo3) {
  ASSERT_EQ(endcap.getCassette(3, 0, 0), 0);
  ASSERT_EQ(endcap.getCassette(3, 31, 15), 0);

  ASSERT_EQ(endcap.getCassette(3, 32, 16), 1);
  ASSERT_EQ(endcap.getCassette(3, 47, 31), 1);

  ASSERT_EQ(endcap.getCassette(3, 0, 32), 2);
  ASSERT_EQ(endcap.getCassette(3, 31, 47), 2);

  ASSERT_EQ(endcap.getCassette(3, 32, 48), 3);
  ASSERT_EQ(endcap.getCassette(3, 47, 63), 3);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
