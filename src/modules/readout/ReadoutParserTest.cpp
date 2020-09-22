// Copyright (C) 2017-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for ReadoutParser
///
//===----------------------------------------------------------------------===//

#include <readout/ReadoutParser.h>
#include <readout/ReadoutParserTestData.h>
#include <test/TestBase.h>

class ReadoutTest : public TestBase {
protected:
  ReadoutParser RdOut;
  const int DataType{0x30};
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ReadoutTest, Constructor) {
  ASSERT_EQ(RdOut.Stats.ErrorBuffer, 0);
  ASSERT_EQ(RdOut.Stats.ErrorSize, 0);
  ASSERT_EQ(RdOut.Stats.ErrorVersion, 0);
  ASSERT_EQ(RdOut.Stats.ErrorTypeSubType, 0);
  ASSERT_EQ(RdOut.Stats.ErrorSeqNum, 0);
}

// nullptr as buffer
TEST_F(ReadoutTest, ErrorBufferPtr) {
  auto Res = RdOut.validate(0, 100, DataType);
  ASSERT_EQ(Res, -ReadoutParser::EBUFFER);
  ASSERT_EQ(RdOut.Stats.ErrorBuffer, 1);
}

// size is 0
TEST_F(ReadoutTest, ErrorBufferSize) {
  auto Res = RdOut.validate((char *)100, 0, DataType);
  ASSERT_EQ(Res, -ReadoutParser::EBUFFER);
  ASSERT_EQ(RdOut.Stats.ErrorBuffer, 1);
}

TEST_F(ReadoutTest, HeaderLTMin) {
  auto Res = RdOut.validate((char *)&ErrCookie[0], 3, ReadoutParser::Loki4Amp);
  ASSERT_EQ(Res, -ReadoutParser::ESIZE);
  ASSERT_EQ(RdOut.Stats.ErrorSize, 1);
}

TEST_F(ReadoutTest, HeaderGTMax) {
  auto Res = RdOut.validate((char *)&ErrCookie[0], 8973, DataType);
  ASSERT_EQ(Res, -ReadoutParser::ESIZE);
  ASSERT_EQ(RdOut.Stats.ErrorSize, 1);
}

TEST_F(ReadoutTest, ErrorCookie) {
  auto Res = RdOut.validate((char *)&ErrCookie[0], ErrCookie.size(), DataType);
  ASSERT_EQ(Res, -ReadoutParser::EHEADER);
  ASSERT_EQ(RdOut.Stats.ErrorVersion, 1);
}

TEST_F(ReadoutTest, ErrorVersion) {
  auto Res = RdOut.validate((char *)&ErrVersion[0], ErrVersion.size(), DataType);
  ASSERT_EQ(Res, -ReadoutParser::EHEADER);
  ASSERT_EQ(RdOut.Stats.ErrorVersion, 1);
}

TEST_F(ReadoutTest, OkVersion) {
  unsigned int Errors{0};
  unsigned int MinSize{4 + PAD_SIZE};
  for (unsigned int Size = MinSize; Size < OkVersion.size(); Size++) {
    Errors++;
    auto Res = RdOut.validate((char *)&OkVersion[0], Size, DataType);
    ASSERT_EQ(Res, -ReadoutParser::ESIZE);
    ASSERT_EQ(RdOut.Stats.ErrorSize, Errors);
    ASSERT_EQ(RdOut.Packet.DataPtr, nullptr);
  }
  auto Res = RdOut.validate((char *)&OkVersion[0], OkVersion.size(), DataType);
  ASSERT_EQ(Res, ReadoutParser::OK);
  ASSERT_EQ(RdOut.Packet.DataPtr, (char *)(&OkVersion[0] + sizeof(ReadoutParser::PacketHeaderV0)));
  ASSERT_EQ(RdOut.Packet.DataLength, 0);
}


TEST_F(ReadoutTest, SeqNumbers) {
  ASSERT_EQ(RdOut.Stats.ErrorSeqNum, 0);

  RdOut.validate((char *)&OkVersion[0], OkVersion.size(), DataType);
  ASSERT_EQ(RdOut.Stats.ErrorSeqNum, 1);

  RdOut.validate((char *)&OkVersionNextSeq[0], OkVersionNextSeq.size(), DataType);
  ASSERT_EQ(RdOut.Stats.ErrorSeqNum, 1);

  RdOut.validate((char *)&OkVersionNextSeq[0], OkVersionNextSeq.size(), DataType);
  ASSERT_EQ(RdOut.Stats.ErrorSeqNum, 2);
}

TEST_F(ReadoutTest, BadReadoutType) {
  auto Res = RdOut.validate((char *)&OkThreeLokiReadouts[0], OkThreeLokiReadouts.size(), 0xff);
  ASSERT_EQ(Res, -ReadoutParser::EHEADER);
  ASSERT_EQ(RdOut.Stats.ErrorTypeSubType, 1);
}

TEST_F(ReadoutTest, DataLengthMismatch) {
  auto res = RdOut.validate((char *)&OkThreeLokiReadouts[0], OkThreeLokiReadouts.size() - 1, DataType);
  ASSERT_EQ(res, -ReadoutParser::ESIZE);
}



int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
