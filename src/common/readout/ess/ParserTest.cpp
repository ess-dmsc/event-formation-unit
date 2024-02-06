// Copyright (C) 2017-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for ReadoutParser
///
//===----------------------------------------------------------------------===//

#include <common/readout/ess/Parser.h>
#include <common/readout/ess/ParserTestData.h>
#include <common/testutils/TestBase.h>

namespace ESSReadout {

class ReadoutTest : public TestBase {
protected:
  Parser RdOut;
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
  ASSERT_EQ(RdOut.Stats.ErrorOutputQueue, 0);
  ASSERT_EQ(RdOut.Stats.ErrorTimeHigh, 0);
  ASSERT_EQ(RdOut.Stats.ErrorTimeFrac, 0);
  ASSERT_EQ(RdOut.Stats.Version0Header, 0);
  ASSERT_EQ(RdOut.Stats.Version1Header, 0);
}

// nullptr as buffer
TEST_F(ReadoutTest, ErrorBufferPtr) {
  auto Res = RdOut.validate(0, 100, DataType);
  ASSERT_EQ(Res, -Parser::EBUFFER);
  ASSERT_EQ(RdOut.Stats.ErrorBuffer, 1);
}

// size is 0
TEST_F(ReadoutTest, ErrorBufferSize) {
  auto Res = RdOut.validate((char *)100, 0, DataType);
  ASSERT_EQ(Res, -Parser::EBUFFER);
  ASSERT_EQ(RdOut.Stats.ErrorBuffer, 1);
}

TEST_F(ReadoutTest, HeaderLTMin) {
  auto Res = RdOut.validate((char *)&ErrCookie[0], 3, Parser::LOKI);
  ASSERT_EQ(Res, -Parser::ESIZE);
  ASSERT_EQ(RdOut.Stats.ErrorSize, 1);
}

TEST_F(ReadoutTest, HeaderGTMax) {
  auto Res = RdOut.validate((char *)&ErrCookie[0], 8973, DataType);
  ASSERT_EQ(Res, -Parser::ESIZE);
  ASSERT_EQ(RdOut.Stats.ErrorSize, 1);
}

TEST_F(ReadoutTest, ErrorPad) {
  auto Res = RdOut.validate((char *)&ErrPad[0], ErrPad.size(), DataType);
  ASSERT_EQ(Res, -Parser::EHEADER);
  ASSERT_EQ(RdOut.Stats.ErrorPad, 1);
}

TEST_F(ReadoutTest, ErrorCookie) {
  auto Res = RdOut.validate((char *)&ErrCookie[0], ErrCookie.size(), DataType);
  ASSERT_EQ(Res, -Parser::EHEADER);
  ASSERT_EQ(RdOut.Stats.ErrorCookie, 1);
}

TEST_F(ReadoutTest, ErrVersion) {
  auto Res =
      RdOut.validate((char *)&ErrVersion[0], ErrVersion.size(), DataType);
  ASSERT_EQ(Res, -Parser::EHEADER);
  ASSERT_EQ(RdOut.Stats.ErrorVersion, 1);
}

TEST_F(ReadoutTest, V0HeaderCount) {
  auto Res = RdOut.validate((char *)&OkVersionV0[0], OkVersionV0.size() + 8, DataType);
  ASSERT_EQ(Res, -Parser::OK);
  ASSERT_EQ(RdOut.Stats.Version0Header, 1);
}

TEST_F(ReadoutTest, V1HeaderCount) {
  auto Res = RdOut.validate((char *)&OkVersionV1[0], OkVersionV1.size() + 8, DataType);
  ASSERT_EQ(Res, -Parser::OK);
  ASSERT_EQ(RdOut.Stats.Version1Header, 1);
}

TEST_F(ReadoutTest, ErrMaxOutputQueue) {
  auto Res = RdOut.validate((char *)&ErrMaxOutputQueue[0],
                            ErrMaxOutputQueue.size(), DataType);
  ASSERT_EQ(Res, -Parser::EHEADER);
  ASSERT_EQ(RdOut.Stats.ErrorOutputQueue, 1);
}

TEST_F(ReadoutTest, OkV0Version) {
  unsigned int Errors{0};
  unsigned int MinSize{7};
  for (unsigned int Size = MinSize; Size < OkVersionV0.size(); Size++) {
    Errors++;
    auto Res = RdOut.validate((char *)&OkVersionV0[0], Size, DataType);
    ASSERT_EQ(Res, -Parser::ESIZE);
    ASSERT_EQ(RdOut.Stats.ErrorSize, Errors);
    ASSERT_EQ(RdOut.Packet.DataPtr, nullptr);
  }
  auto Res = RdOut.validate((char *)&OkVersionV0[0], OkVersionV0.size(), DataType);
  ASSERT_EQ(Res, Parser::OK);
  ASSERT_EQ(RdOut.Packet.DataPtr,
            (char *)(&OkVersionV0[0] + sizeof(Parser::PacketHeaderV0)));
  ASSERT_EQ(RdOut.Packet.DataLength, 0);
  ASSERT_EQ(RdOut.Stats.ErrorOutputQueue, 0);
}

TEST_F(ReadoutTest, OkV1Version) {
  unsigned int Errors{0};
  unsigned int MinSize{7};
  for (unsigned int Size = MinSize; Size < OkVersionV1.size(); Size++) {
    Errors++;
    auto Res = RdOut.validate((char *)&OkVersionV1[0], Size, DataType);
    ASSERT_EQ(Res, -Parser::ESIZE);
    ASSERT_EQ(RdOut.Stats.ErrorSize, Errors);
    ASSERT_EQ(RdOut.Packet.DataPtr, nullptr);
  }
  auto Res = RdOut.validate((char *)&OkVersionV1[0], OkVersionV1.size(), DataType);
  ASSERT_EQ(Res, Parser::OK);
  ASSERT_EQ(RdOut.Packet.DataPtr,
            (char *)(&OkVersionV1[0] + sizeof(Parser::PacketHeaderV1)));
  ASSERT_EQ(RdOut.Packet.DataLength, 0);
  ASSERT_EQ(RdOut.Stats.ErrorOutputQueue, 0);
}

TEST_F(ReadoutTest, SeqNumbers) {
  ASSERT_EQ(RdOut.Stats.ErrorSeqNum, 0);

  RdOut.validate((char *)&OkVersionV0[0], OkVersionV0.size(), DataType);
  ASSERT_EQ(RdOut.Stats.ErrorSeqNum, 1);

  RdOut.validate((char *)&OkVersionNextSeq[0], OkVersionNextSeq.size(),
                 DataType);
  ASSERT_EQ(RdOut.Stats.ErrorSeqNum, 1);

  RdOut.validate((char *)&OkVersionNextSeq[0], OkVersionNextSeq.size(),
                 DataType);
  ASSERT_EQ(RdOut.Stats.ErrorSeqNum, 2);
}

TEST_F(ReadoutTest, BadReadoutType) {
  auto Res = RdOut.validate((char *)&OkThreeLokiReadouts[0],
                            OkThreeLokiReadouts.size(), 0xff);
  ASSERT_EQ(Res, -Parser::EHEADER);
  ASSERT_EQ(RdOut.Stats.ErrorTypeSubType, 1);
}

TEST_F(ReadoutTest, DataLengthMismatch) {
  auto res = RdOut.validate((char *)&OkThreeLokiReadouts[0],
                            OkThreeLokiReadouts.size() - 1, DataType);
  ASSERT_EQ(res, -Parser::ESIZE);
}

TEST_F(ReadoutTest, PulseTimeFracError) {
  auto res = RdOut.validate((char *)&ErrPulseTimeFrac[0],
                            ErrPulseTimeFrac.size(), DataType);
  ASSERT_EQ(res, -Parser::EHEADER);
  ASSERT_EQ(RdOut.Stats.ErrorTimeFrac, 1);
}

TEST_F(ReadoutTest, PrevPulseTimeFracError) {
  auto res = RdOut.validate((char *)&ErrPrevPulseTimeFrac[0],
                            ErrPrevPulseTimeFrac.size(), DataType);
  ASSERT_EQ(res, -Parser::EHEADER);
  ASSERT_EQ(RdOut.Stats.ErrorTimeFrac, 1);
}

TEST_F(ReadoutTest, MaxPulseTimeError) {
  auto res = RdOut.validate((char *)&ErrMaxPulseTime[0], ErrMaxPulseTime.size(),
                            DataType);
  ASSERT_EQ(res, -Parser::EHEADER);
  ASSERT_EQ(RdOut.Stats.ErrorTimeHigh, 1);
}

} // namespace ESSReadout

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
