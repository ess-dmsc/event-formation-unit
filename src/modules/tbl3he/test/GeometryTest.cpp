// Copyright (C) 2024 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <memory>
#include <tbl3he/geometry/Tbl3HeGeometry.h>

auto ValidConfig = R"(
  {
    "Detector": "tbl3he",
    "MaxRing": 11,
    "Resolution": 100,
    "MaxGroup": 7,
    "MaxPulseTimeNS" : 71428600,
    "MaxTOFNS" : 500000000,

    "NumOfFENs" : 2,

    "MinValidAmplitude" : 0,

    "Topology" : [
       {"Ring" : 10, "FEN" : 0, "Bank" : 0},
       {"Ring" :  9, "FEN" : 0, "Bank" : 1}
    ]
  }
)"_json;

using namespace Caen;

class Tbl3HeGeometryTest : public TestBase {
protected:
  Statistics Stats;
  std::unique_ptr<Tbl3HeGeometry> geom;
  Config CaenConfiguration;

  void SetUp() override {
    CaenConfiguration.CaenParms.Resolution = 100;
    CaenConfiguration.CaenParms.MaxGroup = 7;
    geom = std::make_unique<Tbl3HeGeometry>(Stats, CaenConfiguration);

    geom->CaenCDCalibration.Parms.Groups = 8;
    // Make nullcalibration for the eight groups
    for (int i = 0; i < 8; i++) {
      geom->CaenCDCalibration.Intervals.push_back({{0.00, 1.00}});
      geom->CaenCDCalibration.Calibration.push_back({{0.0, 0.0, 0.0, 0.0}});
    }

    CaenConfiguration.Tbl3HeConf.TopologyMapPtr.reset(
        new HashMap2D<Caen::Tbl3HeConfig::Topology>(2));
    auto topo = std::make_unique<Caen::Tbl3HeConfig::Topology>(0);
    CaenConfiguration.Tbl3HeConf.TopologyMapPtr->add(0, 0, topo);
    topo = std::make_unique<Caen::Tbl3HeConfig::Topology>(1);
    CaenConfiguration.Tbl3HeConf.TopologyMapPtr->add(1, 0, topo);
  }
  void TearDown() override {}
};

TEST_F(Tbl3HeGeometryTest, Constructor) {
  ASSERT_EQ(geom->getCaenCounters().AmplitudeZero, 0);
  ASSERT_EQ(geom->getBaseCounters().RingErrors, 0);
  ASSERT_EQ(geom->getBaseCounters().FENErrors, 0);
  ASSERT_EQ(geom->getCaenCounters().GroupErrors, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeLow, 0);
}

TEST_F(Tbl3HeGeometryTest, ValidateReadouts) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  // Check valid FiberIds
  for (int FiberId = 0; FiberId < 24; FiberId++) {
    for (int FENId = 0; FENId < 12; FENId++) {
      readout.FiberId = FiberId;
      readout.FENId = FENId;
      if ((FiberId < 4) and (FENId < 1)) {
        ASSERT_EQ(geom->validateReadoutData(readout), true);
      } else {
        ASSERT_EQ(geom->validateReadoutData(readout), false);
      }
    }
  }
  ASSERT_EQ(geom->getBaseCounters().TopologyError, 24 * 12 - 4);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeLow, 0);
  
  ASSERT_EQ(geom->getCaenCounters().GroupErrors, 0);
  ASSERT_EQ(geom->getBaseCounters().RingErrors, 0);
  ASSERT_EQ(geom->getBaseCounters().FENErrors, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeLow, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeZero, 0);
}

TEST_F(Tbl3HeGeometryTest, ValidateReadoutsGroup) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  // Check valid Groups
  for (uint i = 0; i <= 23; i++) {
    readout.Group = i;
    if (i <= CaenConfiguration.CaenParms.MaxGroup) {
      ASSERT_EQ(geom->validateReadoutData(readout), true);
    } else {
      ASSERT_EQ(geom->validateReadoutData(readout), false);
    }
  }

  ASSERT_EQ(geom->getCaenCounters().GroupErrors, 16);

  ASSERT_EQ(geom->getBaseCounters().RingErrors, 0);
  ASSERT_EQ(geom->getBaseCounters().FENErrors, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeLow, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeZero, 0);
}

TEST_F(Tbl3HeGeometryTest, CalcPixelBadAmpl) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout), 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeZero, 1);

  ASSERT_EQ(geom->getCaenCounters().GroupErrors, 0);
  ASSERT_EQ(geom->getBaseCounters().RingErrors, 0);
  ASSERT_EQ(geom->getBaseCounters().FENErrors, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeLow, 0);
}

TEST_F(Tbl3HeGeometryTest, CalcPixelOutOfRange) {
  //                              R  F               G     A    B
  DataParser::CaenReadout readout{3, 0, 0, 0, 0, 0, 10, 0, 10, 10, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout), 0);

  ASSERT_EQ(geom->getCaenCounters().GroupErrors, 0);
  ASSERT_EQ(geom->getBaseCounters().RingErrors, 0);
  ASSERT_EQ(geom->getBaseCounters().FENErrors, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeLow, 0);
}

TEST_F(Tbl3HeGeometryTest, CalcPixelSelectedOK) {
  //                               R  F              G     A   B  C  D
  DataParser::CaenReadout readout1{0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout1), 1);

  DataParser::CaenReadout readout2{0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout2), 100);
  //                               R  F              G      A  B  C  D
  DataParser::CaenReadout readout3{1, 0, 0, 0, 0, 0, 7, 0, 0, 10, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout3), 701);

  DataParser::CaenReadout readout4{1, 0, 0, 0, 0, 0, 7, 0, 10, 0, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout4), 800);

  ASSERT_EQ(geom->getCaenCounters().GroupErrors, 0);
  ASSERT_EQ(geom->getBaseCounters().RingErrors, 0);
  ASSERT_EQ(geom->getBaseCounters().FENErrors, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeZero, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeLow, 0);
}

TEST_F(Tbl3HeGeometryTest, ErrorAmplitudeLow) {
  CaenConfiguration.Tbl3HeConf.Params.MinValidAmplitude = 1;
  //                               R  F              G     A   B  C  D
  DataParser::CaenReadout readout1{0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout1), 0);

  ASSERT_EQ(geom->getCaenCounters().GroupErrors, 0);
  ASSERT_EQ(geom->getBaseCounters().RingErrors, 0);
  ASSERT_EQ(geom->getBaseCounters().FENErrors, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeZero, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeLow, 1);
}

TEST_F(Tbl3HeGeometryTest, ErrorMaxRing) {
  //                               R  F              G     A   B  C  D
  DataParser::CaenReadout readout{24, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0};
  ASSERT_EQ(geom->validateReadoutData(readout), false);

  ASSERT_EQ(geom->getCaenCounters().GroupErrors, 0);
  ASSERT_EQ(geom->getBaseCounters().RingErrors, 1);
  ASSERT_EQ(geom->getBaseCounters().FENErrors, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeZero, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeLow, 0);
}

TEST_F(Tbl3HeGeometryTest, OutsideUnitInterval) {
  CaenConfiguration.Tbl3HeConf.Params.MinValidAmplitude = -100;
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 10, -1, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout), 0);
}

TEST_F(Tbl3HeGeometryTest, Serialisers) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0};
  ASSERT_EQ(geom->calcSerializer(readout), 0);
  readout.FiberId = 1;
  ASSERT_EQ(geom->calcSerializer(readout), 0);
  readout.FiberId = 2;
  ASSERT_EQ(geom->calcSerializer(readout), 1);
  readout.FiberId = 3;
  ASSERT_EQ(geom->calcSerializer(readout), 1);
}

// Pretty ugly and not easily testable, but it works.
TEST_F(Tbl3HeGeometryTest, OkFromConfigFile) {
  CaenConfiguration.setRoot(ValidConfig);
  CaenConfiguration.parseConfig();
  Tbl3HeGeometry g(Stats, CaenConfiguration);

  // Create nullcalibration profile for the eight Groups (tubes)
  g.CaenCDCalibration.Parms.Groups = 8;
  for (int group = 0; group < 8; group++) {
    g.CaenCDCalibration.Intervals.push_back({{0.00, 1.00}});
    g.CaenCDCalibration.Calibration.push_back({{0.0, 0.0, 0.0, 0.0}});
  }

  DataParser::CaenReadout readout;
  readout.FENId = 0;
  readout.DataLength = 0; // not used for geometry
  readout.TimeHigh = 0;   // not used for geometry
  readout.TimeLow = 0;    // not used for geometry
  readout.FlagsOM = 0;    // not used for geometry
  readout.Unused = 0;     // not used for geometry
  readout.AmpC = 0;
  readout.AmpD = 0;

  // Here goes, ...
  readout.FiberId = 20; // Ring 10
  readout.Group = 0;    // first Tube
  readout.AmpA = 0;
  readout.AmpB = 10;
  ASSERT_EQ(g.calcPixel(readout), 1);

  readout.AmpA = 10;
  readout.AmpB = 0;
  ASSERT_EQ(g.calcPixel(readout), 100);

  readout.Group = 3; // fourth tube
  ASSERT_EQ(g.calcPixel(readout), 400);

  readout.FiberId = 18; // Ring 9
  readout.Group = 0;
  readout.AmpA = 0;
  readout.AmpB = 10;
  ASSERT_EQ(g.calcPixel(readout), 401);

  readout.AmpA = 10;
  readout.AmpB = 0;
  ASSERT_EQ(g.calcPixel(readout), 500);

  readout.Group = 3; // fourth tube
  ASSERT_EQ(g.calcPixel(readout), 800);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
