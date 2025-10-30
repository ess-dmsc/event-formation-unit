
// Copyright (C) 2023 - 2024 European Spallation Source ERIC, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for PADetector
///
//===----------------------------------------------------------------------===//
#include <common/Statistics.h>
#include <common/testutils/TestBase.h>
#include <dream/geometry/PADetector.h>
#include <memory>

using namespace Dream;

class PADetectorTest : public TestBase {
protected:
  Statistics Stats;
  DataParser::CDTReadout Data{0, 0, 0, 0, 0, 0, 0, 16, 16};
  Config::ModuleParms Parms{false, Config::ModuleType::PA, {0}, {0}};
  std::unique_ptr<PADetector> detb;

  void SetUp() override {
    detb = std::make_unique<PADetector>(Stats, 256, 512);
  }
};

TEST_F(PADetectorTest, InvalidSector) {
  Parms.P1.Sector = 7;
  ASSERT_NE(detb->calcPixelId(Parms, Data), 0);
  Parms.P1.Sector = 8;
  ASSERT_EQ(detb->calcPixelId(Parms, Data), 0);
  
  const auto &counters = detb->getPADetectorCounters();
  ASSERT_EQ(counters.MaxSectorErrors, 1);
  ASSERT_EQ(counters.CassetteIdErrors, 0);
  ASSERT_EQ(counters.CounterErrors, 0);
  ASSERT_EQ(counters.WireErrors, 0);
  ASSERT_EQ(counters.StripErrors, 0);
}

TEST_F(PADetectorTest, ValidSector) {
  for (uint8_t Sector = 0; Sector < 8; Sector++) {
    Parms.P1.Sector = Sector;
    ASSERT_NE(detb->calcPixelId(Parms, Data), 0);
  }
}

TEST_F(PADetectorTest, GetXInvalidCassette) {
  uint8_t Sector{0};
  uint8_t Counter{0};
  for (int Cassette = 0; Cassette < 16; Cassette++) {
    //                      Cassette
    ASSERT_NE(detb->getX(Sector, Cassette, Counter), -1);
  }
  ASSERT_EQ(detb->getX(Sector, 16, Counter), -1);
  
  const auto &counters = detb->getPADetectorCounters();
  ASSERT_EQ(counters.MaxSectorErrors, 0);
  ASSERT_EQ(counters.CassetteIdErrors, 1);
  ASSERT_EQ(counters.CounterErrors, 0);
  ASSERT_EQ(counters.WireErrors, 0);
  ASSERT_EQ(counters.StripErrors, 0);
}

TEST_F(PADetectorTest, GetXInvalidCounter) {
  uint8_t Sector{0};
  uint8_t Cassette{1};
  //                            Sumo  Cassette  Counter
  ASSERT_NE(detb->getX(Sector, Cassette, 0), -1);
  ASSERT_NE(detb->getX(Sector, Cassette, 1), -1);
  ASSERT_EQ(detb->getX(Sector, Cassette, 2), -1);
  
  const auto &counters = detb->getPADetectorCounters();
  ASSERT_EQ(counters.MaxSectorErrors, 0);
  ASSERT_EQ(counters.CassetteIdErrors, 0);
  ASSERT_EQ(counters.CounterErrors, 1);
  ASSERT_EQ(counters.WireErrors, 0);
  ASSERT_EQ(counters.StripErrors, 0);
}

TEST_F(PADetectorTest, GetYInvalidWire) {
  //                    Wire, Strip
  ASSERT_NE(detb->getY(0, 0), -1);
  ASSERT_NE(detb->getY(15, 0), -1);
  ASSERT_EQ(detb->getY(16, 0), -1);

  ASSERT_NE(detb->getY(0, 15), -1);
  ASSERT_NE(detb->getY(15, 15), -1);
  ASSERT_EQ(detb->getY(16, 15), -1);
  
  const auto &counters = detb->getPADetectorCounters();
  ASSERT_EQ(counters.MaxSectorErrors, 0);
  ASSERT_EQ(counters.CassetteIdErrors, 0);
  ASSERT_EQ(counters.CounterErrors, 0);
  ASSERT_EQ(counters.WireErrors, 2);
  ASSERT_EQ(counters.StripErrors, 0);
}

TEST_F(PADetectorTest, GetYInvalidStrip) {
  //               Wire, Strip
  ASSERT_NE(detb->getY(0, 0), -1);
  ASSERT_NE(detb->getY(0, 31), -1);
  ASSERT_EQ(detb->getY(0, 32), -1);

  ASSERT_NE(detb->getY(15, 0), -1);
  ASSERT_NE(detb->getY(15, 31), -1);
  ASSERT_EQ(detb->getY(15, 32), -1);
  
  const auto &counters = detb->getPADetectorCounters();
  ASSERT_EQ(counters.MaxSectorErrors, 0);
  ASSERT_EQ(counters.CassetteIdErrors, 0);
  ASSERT_EQ(counters.CounterErrors, 0);
  ASSERT_EQ(counters.WireErrors, 0);
  ASSERT_EQ(counters.StripErrors, 2);
}

TEST_F(PADetectorTest, TestingLogicalGeometryCorners) {
  //                 sec cas ctr
  ASSERT_EQ(detb->getX(0, 0, 0), 0);    // left
  ASSERT_EQ(detb->getX(7, 15, 1), 255); // right

  //                wire strip
  ASSERT_EQ(detb->getY(0, 0), 0);     // top
  ASSERT_EQ(detb->getY(15, 31), 511); // bottom
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
