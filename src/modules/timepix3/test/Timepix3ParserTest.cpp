// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <timepix3/readout/DataParser.h>

using namespace Timepix3;

// clang-format off
std::vector<uint8_t> SinglePixelReadout{
    // Single readout
    0x91, 0xc6, 0x30, 0x80,
    0x8b, 0xa8, 0x3a, 0xbf
};

std::vector<uint8_t> TooShort{
    0x00, 0x01
};

std::vector<uint8_t> SingleTDCReadout{
    // Single readout
    0xc0, 0x42, 0x9f, 0xdd,
    0xa4, 0x7e, 0x8b, 0x6f
};

std::vector<uint8_t> SingleEVRReadout{
  0x01, 0x45, 0x53, 0x53,
  0x2a, 0xc0, 0x08, 0x00,
  0xcc, 0xa1, 0x3f, 0x64,
  0xdd, 0xc9, 0x9a, 0x3b,
  0xcb, 0xa1, 0x3f, 0x64,
  0xdd, 0xc9, 0x9a, 0x3b
};
// clang-format on

class Timepix3ParserTest : public TestBase {
protected:
  struct Counters counters;

  DataParser *Timepix3Parser;

  void SetUp() override { Timepix3Parser = new DataParser(counters); }
  void TearDown() override {}
};

// Test cases below
TEST_F(Timepix3ParserTest, Constructor) { DataParser Timepix3Parser(counters); }

TEST_F(Timepix3ParserTest, SinglePixelReadout) {
  auto Res = Timepix3Parser->parse((char *)SinglePixelReadout.data(),
                                   SinglePixelReadout.size());
  EXPECT_EQ(Res, 1);
}

TEST_F(Timepix3ParserTest, SingleTDCReadout) {
  auto Res = Timepix3Parser->parse((char *)SingleTDCReadout.data(),
                                   SingleTDCReadout.size());
  EXPECT_EQ(Res, 1);
}

TEST_F(Timepix3ParserTest, TooShort) {
  auto Res = Timepix3Parser->parse((char *)TooShort.data(), TooShort.size());
  EXPECT_EQ(Res, 0);
}

TEST_F(Timepix3ParserTest, SingleEVRReadout) {
  auto Res = Timepix3Parser->parse((char *)SingleEVRReadout.data(),
                                   SingleEVRReadout.size());
  EXPECT_EQ(Res, 1);
}

int main(int argc, char **argv) {

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  return RetVal;
}
