// Copyright (C) 2024 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/Statistics.h>
#include <common/testutils/TestBase.h>
#include <dream/geometry/Config.h>
#include <dream/geometry/HeimdalGeometry.h>
#include <dream/readout/DataParser.h>
#include <memory>

using namespace Dream;

class HeimdalGeometryTest : public TestBase {
protected:
  DataParser::CDTReadout Readout{0, 0, 0, 0, 0, 0, 0, 70, 70};
  std::unique_ptr<HeimdalGeometry> geometry;
  Statistics Stats;
  Config DreamConfig;

  HeimdalGeometryTest() = default;

  void SetUp() override {

    // Create geometry with config
    geometry = std::make_unique<HeimdalGeometry>(Stats, DreamConfig);
  }

  void TearDown() override {}
};

///\brief only HeimdalMantle is valid
TEST_F(HeimdalGeometryTest, PixelOffsets) {
  ASSERT_EQ(geometry->getPixelOffset(Config::HeimdalMantle), 0);
  ASSERT_EQ(geometry->getPixelOffset(Config::HR), -1);
  ASSERT_EQ(geometry->getPixelOffset(Config::SANS), -1);
  ASSERT_EQ(geometry->getPixelOffset(Config::FwEndCap), -1);
  ASSERT_EQ(geometry->getPixelOffset(Config::BwEndCap), -1);
  ASSERT_EQ(geometry->getPixelOffset(Config::DreamMantle), -1);
}

///\todo only tests (some) x-coordinates
TEST_F(HeimdalGeometryTest, GetPixel) {
  DreamConfig.RMConfig[0][0].Type = Config::ModuleType::HeimdalMantle;
  ASSERT_EQ(geometry->calcPixel<DataParser::CDTReadout>(Readout), 56161); // Cass 0 of first MU
  DreamConfig.RMConfig[0][0].P1.MU = 1;
  ASSERT_EQ(geometry->calcPixel<DataParser::CDTReadout>(Readout), 56161 + 12); // Cass 0 of second MU
  DreamConfig.RMConfig[0][0].P1.MU = 2;
  ASSERT_EQ(geometry->calcPixel<DataParser::CDTReadout>(Readout), 56161 + 24); // Cass 0 of third MU
  DreamConfig.RMConfig[0][0].P1.MU = 11;
  DreamConfig.RMConfig[0][0].P2.Cassette = 5;
  ASSERT_EQ(geometry->calcPixel<DataParser::CDTReadout>(Readout), 56161 + 142); // last Cass of last MU
}

TEST_F(HeimdalGeometryTest, GetPixelBadType) {
  DreamConfig.RMConfig[0][0].Type = Config::ModuleType::DreamMantle;
  ASSERT_EQ(geometry->calcPixel<DataParser::CDTReadout>(Readout), 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
