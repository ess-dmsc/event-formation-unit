// Copyright (C) 2020-2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <timepix3/Timepix3Instrument.h>

using namespace Timepix3;

class Timepix3InstrumentTest : public TestBase {
protected:
  struct Counters counters;
  BaseSettings Settings;

  void SetUp() override { Settings.ConfigFile = TIMEPIX_CONFIG; }
  void TearDown() override {}
};

// Test cases below
TEST_F(Timepix3InstrumentTest, Constructor) {
  Timepix3Instrument Timepix3(counters, Settings);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
