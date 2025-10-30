
// Copyright (C) 2021 - 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for Dream geometry
///
//===----------------------------------------------------------------------===//
#include <common/Statistics.h>
#include <common/testutils/TestBase.h>
#include <cstdint>
#include <dream/geometry/SUMO.h>
#include <memory>

// fails InvalidSumo test on CentOS build - false positive
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic pop

using namespace Dream;

class DreamGeometryTest : public TestBase {
protected:
  Statistics Stats;
  DataParser::CDTReadout Data{0, 0, 0, 0, 0, 0, 0, 16, 16};
  Config::ModuleParms Parms{false, Config::ModuleType::BwEndCap, {0}, {0}};
  std::unique_ptr<SUMO> endcap;

  void SetUp() override { endcap = std::make_unique<SUMO>(Stats, 616, 256); }
};

TEST_F(DreamGeometryTest, Constructor) {
  ASSERT_EQ(endcap->calcPixelId(Parms, Data), 0);
}

TEST_F(DreamGeometryTest, InvalidSector) {
  Data.UnitId = 6; // Valid SUMO

  Parms.P1.Sector = 23;
  ASSERT_EQ(endcap->calcPixelId(Parms, Data), 0);
  Parms.P1.Sector = 24;
  ASSERT_EQ(endcap->calcPixelId(Parms, Data), 0);

  // Verify ONLY MaxSectorErrors was incremented, all others are 0
  const auto &counters = endcap->getSUMOCounters();
  ASSERT_EQ(counters.MaxSectorErrors, 2);
  ASSERT_EQ(counters.SumoIdErrors, 0);
  ASSERT_EQ(counters.CassetteIdErrors, 0);
  ASSERT_EQ(counters.CounterErrors, 0);
  ASSERT_EQ(counters.MaxWireErrors, 0);
  ASSERT_EQ(counters.MaxStripErrors, 0);
}

TEST_F(DreamGeometryTest, ValidSector) {
  Data.UnitId = 6;
  for (uint8_t Sector = 0; Sector < 11; Sector++) {
    Parms.P1.Sector = Sector;
    ASSERT_NE(endcap->calcPixelId(Parms, Data), 0);
  }
}

TEST_F(DreamGeometryTest, InvalidSumo) {
  Parms.P1.Sector = 0; // Valid sector

  // Test invalid SUMO IDs by calling getX() directly
  // This ensures we test the SumoIdErrors counter
  uint8_t Sector{0};
  uint8_t Cassette{1};
  uint8_t Counter{0};

  std::vector<int> InvalidSumoIDs{0, 1, 2, 7, 8};
  for (auto const &ID : InvalidSumoIDs) {
    ASSERT_EQ(endcap->getX(Sector, ID, Cassette, Counter), -1);
  }

  // Verify ONLY SumoIdErrors was incremented, all others are 0
  const auto &counters = endcap->getSUMOCounters();
  ASSERT_EQ(counters.MaxSectorErrors, 0);
  ASSERT_EQ(counters.SumoIdErrors, InvalidSumoIDs.size());
  ASSERT_EQ(counters.CassetteIdErrors, 0);
  ASSERT_EQ(counters.CounterErrors, 0);
  ASSERT_EQ(counters.MaxWireErrors, 0);
  ASSERT_EQ(counters.MaxStripErrors, 0);
}

TEST_F(DreamGeometryTest, ValidSumo) {
  std::vector<int> SumoIDs{3, 4, 5, 6};
  for (auto const &ID : SumoIDs) {
    Data.UnitId = ID;
    ASSERT_NE(endcap->calcPixelId(Parms, Data), 0);
  }
}

TEST_F(DreamGeometryTest, GetXInvalidCassette) {
  uint8_t Sector{0};
  uint8_t Counter{0};

  //                            Sumo  Cassette
  ASSERT_NE(endcap->getX(Sector, 3, 0, Counter), -1);
  ASSERT_EQ(endcap->getX(Sector, 3, 4, Counter), -1);
  ASSERT_NE(endcap->getX(Sector, 4, 0, Counter), -1);
  ASSERT_EQ(endcap->getX(Sector, 4, 6, Counter), -1);
  ASSERT_NE(endcap->getX(Sector, 5, 0, Counter), -1);
  ASSERT_EQ(endcap->getX(Sector, 5, 8, Counter), -1);
  ASSERT_NE(endcap->getX(Sector, 6, 0, Counter), -1);
  ASSERT_EQ(endcap->getX(Sector, 6, 10, Counter), -1);

  // Verify ONLY CassetteIdErrors was incremented, all others are 0
  const auto &counters = endcap->getSUMOCounters();
  ASSERT_EQ(counters.MaxSectorErrors, 0);
  ASSERT_EQ(counters.SumoIdErrors, 0);
  ASSERT_EQ(counters.CassetteIdErrors, 4);
  ASSERT_EQ(counters.CounterErrors, 0);
  ASSERT_EQ(counters.MaxWireErrors, 0);
  ASSERT_EQ(counters.MaxStripErrors, 0);
}

TEST_F(DreamGeometryTest, GetXInvalidCounter) {
  uint8_t Sector{0};
  uint8_t Cassette{1};
  //                            Sumo  Cassette  Counter
  ASSERT_NE(endcap->getX(Sector, 3, Cassette, 0), -1);
  ASSERT_NE(endcap->getX(Sector, 3, Cassette, 1), -1);
  ASSERT_EQ(endcap->getX(Sector, 3, Cassette, 2), -1);
  ASSERT_NE(endcap->getX(Sector, 4, Cassette, 0), -1);
  ASSERT_NE(endcap->getX(Sector, 4, Cassette, 1), -1);
  ASSERT_EQ(endcap->getX(Sector, 4, Cassette, 2), -1);
  ASSERT_NE(endcap->getX(Sector, 5, Cassette, 0), -1);
  ASSERT_NE(endcap->getX(Sector, 5, Cassette, 1), -1);
  ASSERT_EQ(endcap->getX(Sector, 5, Cassette, 2), -1);
  ASSERT_NE(endcap->getX(Sector, 6, Cassette, 0), -1);
  ASSERT_NE(endcap->getX(Sector, 6, Cassette, 1), -1);
  ASSERT_EQ(endcap->getX(Sector, 6, Cassette, 2), -1);

  // Verify ONLY CounterErrors was incremented, all others are 0
  const auto &counters = endcap->getSUMOCounters();
  ASSERT_EQ(counters.MaxSectorErrors, 0);
  ASSERT_EQ(counters.SumoIdErrors, 0);
  ASSERT_EQ(counters.CassetteIdErrors, 0);
  ASSERT_EQ(counters.CounterErrors, 4);
  ASSERT_EQ(counters.MaxWireErrors, 0);
  ASSERT_EQ(counters.MaxStripErrors, 0);
}

TEST_F(DreamGeometryTest, GetYInvalidWire) {
  //                    Wire, Strip
  ASSERT_NE(endcap->getY(0, 0), -1);
  ASSERT_NE(endcap->getY(15, 0), -1);
  ASSERT_EQ(endcap->getY(16, 0), -1);

  ASSERT_NE(endcap->getY(0, 15), -1);
  ASSERT_NE(endcap->getY(15, 15), -1);
  ASSERT_EQ(endcap->getY(16, 15), -1);
}

TEST_F(DreamGeometryTest, GetYInvalidWireCounterIncrement) {
  // Test invalid wires (Wire > MaxWire=15)
  ASSERT_EQ(endcap->getY(16, 0), -1);
  ASSERT_EQ(endcap->getY(16, 15), -1);
  ASSERT_EQ(endcap->getY(20, 5), -1);

  // Verify ONLY MaxWireErrors was incremented, all others are 0
  const auto &counters = endcap->getSUMOCounters();
  ASSERT_EQ(counters.MaxSectorErrors, 0);
  ASSERT_EQ(counters.SumoIdErrors, 0);
  ASSERT_EQ(counters.CassetteIdErrors, 0);
  ASSERT_EQ(counters.CounterErrors, 0);
  ASSERT_EQ(counters.MaxWireErrors, 3);
  ASSERT_EQ(counters.MaxStripErrors, 0);
}

TEST_F(DreamGeometryTest, GetYInvalidStrip) {
  //                    Wire, Strip
  ASSERT_NE(endcap->getY(0, 0), -1);
  ASSERT_NE(endcap->getY(0, 15), -1);
  ASSERT_EQ(endcap->getY(0, 16), -1);

  ASSERT_NE(endcap->getY(15, 0), -1);
  ASSERT_NE(endcap->getY(15, 15), -1);
  ASSERT_EQ(endcap->getY(15, 16), -1);
}

TEST_F(DreamGeometryTest, GetYInvalidStripCounterIncrement) {
  // Test invalid strips (Strip > MaxStrip=15)
  ASSERT_EQ(endcap->getY(0, 16), -1);
  ASSERT_EQ(endcap->getY(15, 16), -1);
  ASSERT_EQ(endcap->getY(10, 20), -1);

  // Verify ONLY MaxStripErrors was incremented, all others are 0
  const auto &counters = endcap->getSUMOCounters();
  ASSERT_EQ(counters.MaxSectorErrors, 0);
  ASSERT_EQ(counters.SumoIdErrors, 0);
  ASSERT_EQ(counters.CassetteIdErrors, 0);
  ASSERT_EQ(counters.CounterErrors, 0);
  ASSERT_EQ(counters.MaxWireErrors, 0);
  ASSERT_EQ(counters.MaxStripErrors, 3);
}

TEST_F(DreamGeometryTest, TestingICD4BoxCorners) {
  // Refer to figure 7 (logical geometry)
  //                    sec su cas ctr
  ASSERT_EQ(endcap->getX(0, 6, 0, 0), 0);        // top left
  ASSERT_EQ(endcap->getX(22, 3, 3, 1), 1287);    // top right
  ASSERT_EQ(endcap->getX(3, 6, 0, 0), 168 + 0);  // box left
  ASSERT_EQ(endcap->getX(3, 3, 3, 1), 168 + 55); // box right

  //                    wir str
  ASSERT_EQ(endcap->getY(0, 3), 48 + 0);   // box top
  ASSERT_EQ(endcap->getY(15, 3), 48 + 15); // box bottom
  ASSERT_EQ(endcap->getY(0, 0), 0);        // top
  ASSERT_EQ(endcap->getY(15, 15), 255);    // bottom
}

// Testing CDT specified mappings
// From dream_voxel_position_relations[3].png
// provided by Daniel Hollain 30 Jan 2023
TEST_F(DreamGeometryTest, CDTMappingSumo6) {
  ASSERT_EQ(endcap->getCassette(6, 0, 0), 0);
  ASSERT_EQ(endcap->getCassette(6, 31, 15), 0);

  ASSERT_EQ(endcap->getCassette(6, 0, 16), 2);
  ASSERT_EQ(endcap->getCassette(6, 32, 16), 1);
  ASSERT_EQ(endcap->getCassette(6, 0, 32), 4);
  ASSERT_EQ(endcap->getCassette(6, 32, 32), 3);
  ASSERT_EQ(endcap->getCassette(6, 0, 48), 6);
  ASSERT_EQ(endcap->getCassette(6, 32, 48), 5);
  ASSERT_EQ(endcap->getCassette(6, 0, 64), 8);
  ASSERT_EQ(endcap->getCassette(6, 32, 64), 7);
  ASSERT_EQ(endcap->getCassette(6, 32, 80), 9);
}

TEST_F(DreamGeometryTest, CDTMappingSumo5) {
  ASSERT_EQ(endcap->getCassette(5, 0, 0), 0);
  ASSERT_EQ(endcap->getCassette(5, 32, 0), 1);
  ASSERT_EQ(endcap->getCassette(5, 0, 16), 2);
  ASSERT_EQ(endcap->getCassette(5, 32, 16), 3);
  ASSERT_EQ(endcap->getCassette(5, 0, 32), 4);
  ASSERT_EQ(endcap->getCassette(5, 32, 32), 5);
  ASSERT_EQ(endcap->getCassette(5, 0, 48), 6);
  ASSERT_EQ(endcap->getCassette(5, 32, 48), 7);
}

TEST_F(DreamGeometryTest, CDTMappingSumo4) {
  ASSERT_EQ(endcap->getCassette(4, 32, 0), 0);
  ASSERT_EQ(endcap->getCassette(4, 0, 16), 1);
  ASSERT_EQ(endcap->getCassette(4, 32, 16), 2);
  ASSERT_EQ(endcap->getCassette(4, 0, 32), 3);
  ASSERT_EQ(endcap->getCassette(4, 32, 32), 4);
  ASSERT_EQ(endcap->getCassette(4, 0, 48), 5);
}

TEST_F(DreamGeometryTest, CDTMappingSumo3) {
  ASSERT_EQ(endcap->getCassette(3, 0, 0), 0);
  ASSERT_EQ(endcap->getCassette(3, 31, 15), 0);

  ASSERT_EQ(endcap->getCassette(3, 32, 16), 1);
  ASSERT_EQ(endcap->getCassette(3, 47, 31), 1);

  ASSERT_EQ(endcap->getCassette(3, 0, 32), 2);
  ASSERT_EQ(endcap->getCassette(3, 31, 47), 2);

  ASSERT_EQ(endcap->getCassette(3, 32, 48), 3);
  ASSERT_EQ(endcap->getCassette(3, 47, 63), 3);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
