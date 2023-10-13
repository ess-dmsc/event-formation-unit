// Copyright (C) 2020 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <caen/CaenInstrument.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>

using namespace Caen;

class CaenInstrumentTest : public TestBase {
protected:
  struct CaenCounters counters;
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
  CaenInstrument Caen(counters, Settings);
}

TEST_F(CaenInstrumentTest, BifrostConstructor) {
  Settings.ConfigFile = BIFROST_CONFIG;
  Settings.CalibFile = BIFROST_CALIB;
  Settings.DetectorName = "bifrost";
  CaenInstrument Caen(counters, Settings);
}

TEST_F(CaenInstrumentTest, BifrostConstructorNoCalib) {
  Settings.ConfigFile = BIFROST_CONFIG;
  Settings.CalibFile = "";
  Settings.DetectorName = "bifrost";
  ASSERT_ANY_THROW(CaenInstrument Caen(counters, Settings));
}

TEST_F(CaenInstrumentTest, CspecConstructor) {
  Settings.ConfigFile = CSPEC_CONFIG;
  Settings.CalibFile = CSPEC_CALIB;
  Settings.DetectorName = "cspec";
  CaenInstrument Caen(counters, Settings);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
