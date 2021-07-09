// Copyright (C) 2017-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for ReadoutParser
///
//===----------------------------------------------------------------------===//

#include <readout/vmm3/VMM3Parser.h>
#include <test/TestBase.h>

class VMM3ReadoutTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};


TEST_F(VMM3ReadoutTest, Constructor) {
  ASSERT_EQ(0, 0);
}

// // nullptr as buffer
// TEST_F(ReadoutTest, ErrorBufferPtr) {
//   auto Res = RdOut.validate(0, 100, DataType);
//   ASSERT_EQ(Res, -ReadoutParser::EBUFFER);
//   ASSERT_EQ(RdOut.Stats.ErrorBuffer, 1);
// }
//
// // size is 0
// TEST_F(ReadoutTest, ErrorBufferSize) {
//   auto Res = RdOut.validate((char *)100, 0, DataType);
//   ASSERT_EQ(Res, -ReadoutParser::EBUFFER);
//   ASSERT_EQ(RdOut.Stats.ErrorBuffer, 1);
// }
//
// TEST_F(ReadoutTest, HeaderLTMin) {
//   auto Res = RdOut.validate((char *)&ErrCookie[0], 3, ReadoutParser::Loki4Amp);
//   ASSERT_EQ(Res, -ReadoutParser::ESIZE);
//   ASSERT_EQ(RdOut.Stats.ErrorSize, 1);
// }
//
// TEST_F(ReadoutTest, HeaderGTMax) {
//   auto Res = RdOut.validate((char *)&ErrCookie[0], 8973, DataType);
//   ASSERT_EQ(Res, -ReadoutParser::ESIZE);
//   ASSERT_EQ(RdOut.Stats.ErrorSize, 1);
// }

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
