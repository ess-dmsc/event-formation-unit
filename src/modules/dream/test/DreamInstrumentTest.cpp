// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <dream/DreamInstrument.h>
#include <test/TestBase.h>

using namespace Jalousie;

class DreamInstrumentTest : public TestBase {
protected:
  struct Counters counters;
  DreamSettings ModuleSettings;

  void SetUp() override {}
  void TearDown() override {}
};

/** Test cases below */
TEST_F(DreamInstrumentTest, Constructor) {
  DreamInstrument Dream(counters, ModuleSettings);
  /// \todo Need one or two better inputs here 
  ASSERT_EQ(Dream.calcPixel(0, 0, 0, 0, 0, 0), 0);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
