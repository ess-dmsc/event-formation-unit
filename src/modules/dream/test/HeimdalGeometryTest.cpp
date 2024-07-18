// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <dream/geometry/HeimdalGeometry.h>
#include <dream/readout/DataParser.h>

using namespace Dream;

class HeimdalGeometryTest : public TestBase {
protected:
  DataParser::CDTReadout Readout{0, 0, 0, 0, 0, 0, 0, 0, 0};
  Config::ModuleParms Parms{false, Config::ModuleType::HeimdalMantle, {0}, {0}};
  HeimdalGeometry geometry;
  void SetUp() override {}
  void TearDown() override {}
};

///\brief only HeimdalMantle is valid
TEST_F(HeimdalGeometryTest, PixelOffsets) {
  ASSERT_EQ(geometry.getPixelOffset(Config::HeimdalMantle), 0);
  ASSERT_EQ(geometry.getPixelOffset(Config::HR), -1);
  ASSERT_EQ(geometry.getPixelOffset(Config::SANS), -1);
  ASSERT_EQ(geometry.getPixelOffset(Config::FwEndCap), -1);
  ASSERT_EQ(geometry.getPixelOffset(Config::BwEndCap), -1);
  ASSERT_EQ(geometry.getPixelOffset(Config::DreamMantle), -1);
}

///\todo only tests (some) x-coordinates
TEST_F(HeimdalGeometryTest, GetPixel) {
  Parms.Type = Config::ModuleType::HeimdalMantle;
  ASSERT_EQ(geometry.getPixel(Parms, Readout), 1); // Cass 0 of first MU
  Parms.P1.MU = 1;
  ASSERT_EQ(geometry.getPixel(Parms, Readout), 13); // Cass 0 of second MU
  Parms.P1.MU = 2;
  ASSERT_EQ(geometry.getPixel(Parms, Readout), 25); // Cass 0 of third MU
  Parms.P1.MU = 11;
  Parms.P2.Cassette = 5;
  ASSERT_EQ(geometry.getPixel(Parms, Readout), 143); // last Cass of last MU
}

TEST_F(HeimdalGeometryTest, GetPixelBadType) {
  Parms.Type = Config::ModuleType::DreamMantle;
  ASSERT_EQ(geometry.getPixel(Parms, Readout), 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
