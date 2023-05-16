// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <timepix3/readout/DataParser.h>

using namespace Timepix3;

class Timepix3ParserTest : public TestBase {
protected:
  struct Counters counters;

  DataParser *timepix3;

  void SetUp() override {
     timepix3 = new DataParser(counters);
   }
  void TearDown() override {}
};

// Test cases below
TEST_F(Timepix3ParserTest, Constructor) {
  DataParser Timepix3(counters);
}

// TEST_F(Timepix3ParserTest, SingleGoodReadout) {
//   auto Res = timepix3->Timepix3Parser.parse((char *)SingleGoodReadout.data(), SingleGoodReadout.size());
//   ASSERT_EQ(Res, 1);
//   ASSERT_EQ(counters.PixelReadouts, 1);

//   timepix3->processReadouts();

//   // ASSERT_EQ(counters.Events, 1);
// }

int main(int argc, char **argv) {

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  return RetVal;
}
