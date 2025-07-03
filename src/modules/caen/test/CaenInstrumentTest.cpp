// Copyright (C) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include "common/Statistics.h"
#include "common/readout/ess/Parser.h"
#include <caen/CaenInstrument.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>

using namespace Caen;

class CaenInstrumentTest : public TestBase {
protected:
  struct CaenCounters counters;
  Statistics stats;
  ESSReadout::Parser parser{stats};
  BaseSettings Settings;

  void SetUp() override {
    Settings.DetectorName = "loki";
    Settings.ConfigFile = LOKI_CONFIG;
  }
  void TearDown() override {}
};

// Test cases below
TEST_F(CaenInstrumentTest, LokiConstructor) {
  Settings.CalibFile = LOKI_CALIB;
  CaenInstrument Caen(counters, Settings, parser);
}

TEST_F(CaenInstrumentTest, BifrostConstructor) {
  Settings.ConfigFile = BIFROST_CONFIG;
  Settings.CalibFile = BIFROST_CALIB;
  Settings.DetectorName = "bifrost";
  CaenInstrument Caen(counters, Settings, parser);
}

TEST_F(CaenInstrumentTest, BifrostConstructorNoCalib) {
  Settings.ConfigFile = BIFROST_CONFIG;
  Settings.CalibFile = "";
  Settings.DetectorName = "bifrost";
  ASSERT_ANY_THROW(CaenInstrument Caen(counters, Settings, parser));
}

TEST_F(CaenInstrumentTest, CspecConstructor) {
  Settings.ConfigFile = CSPEC_CONFIG;
  Settings.CalibFile = CSPEC_CALIB;
  Settings.DetectorName = "cspec";
  CaenInstrument Caen(counters, Settings, parser);
}

TEST_F(CaenInstrumentTest, Tbl3HeConstructor) {
  Settings.ConfigFile = TBL3HE_CONFIG;
  Settings.CalibFile = TBL3HE_CALIB;
  Settings.DetectorName = "tbl3he";
  CaenInstrument Caen(counters, Settings, parser);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
