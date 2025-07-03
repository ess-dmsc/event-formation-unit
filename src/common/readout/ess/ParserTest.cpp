// Copyright (C) 2017 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for ReadoutParser
///
//===----------------------------------------------------------------------===//

#include <common/Statistics.h>
#include <common/readout/ess/Parser.h>
#include <common/readout/ess/ParserTestData.h>
#include <common/testutils/TestBase.h>

namespace ESSReadout {

class ReadoutTest : public TestBase {
protected:
  Statistics Stats;
  Parser ESSParser{Stats};
  const int DataType{0x30};
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ReadoutTest, Constructor) {
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_BUFFER), 0);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_SIZE), 0);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_VERSION), 0);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_TYPE), 0);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_SEQNO), 0);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_OUTPUT_QUEUE), 0);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_TIMEHIGH), 0);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_TIMEFRAC), 0);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_VERSION_V0), 0);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_VERSION_V1), 0);
}

TEST_F(ReadoutTest, HeaderSizesCalculated) {
  Parser::PacketHeader header =
      Parser::PacketHeader((Parser::PacketHeaderV0 *)(char *)&OkVersionV0[0]);
  EXPECT_EQ(header.getSize(), sizeof(Parser::PacketHeaderV0));

  header =
      Parser::PacketHeader((Parser::PacketHeaderV1 *)(char *)&OkVersionV1[0]);
  EXPECT_EQ(header.getSize(), sizeof(Parser::PacketHeaderV1));
}

// nullptr as buffer
TEST_F(ReadoutTest, ErrorBufferPtr) {
  auto Res = ESSParser.validate(0, 100, DataType);
  ASSERT_EQ(Res, -Parser::EBUFFER);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_BUFFER), 1);
}

// size is 0
TEST_F(ReadoutTest, ErrorBufferSize) {
  auto Res = ESSParser.validate((char *)100, 0, DataType);
  ASSERT_EQ(Res, -Parser::EBUFFER);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_BUFFER), 1);
}

TEST_F(ReadoutTest, HeaderLTMin) {
  auto Res = ESSParser.validate((char *)&ErrCookie[0], 3, DetectorType::LOKI);
  ASSERT_EQ(Res, -Parser::ESIZE);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_SIZE), 1);
}

TEST_F(ReadoutTest, HeaderGTMax) {
  auto Res = ESSParser.validate((char *)&ErrCookie[0], 8973, DataType);
  ASSERT_EQ(Res, -Parser::ESIZE);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_SIZE), 1);
}

TEST_F(ReadoutTest, ErrorPad) {
  auto Res = ESSParser.validate((char *)&ErrPad[0], ErrPad.size(), DataType);
  ASSERT_EQ(Res, -Parser::EHEADER);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_PAD), 1);
}

TEST_F(ReadoutTest, ErrorCookie) {
  auto Res = ESSParser.validate((char *)&ErrCookie[0], ErrCookie.size(), DataType);
  ASSERT_EQ(Res, -Parser::EHEADER);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_COOKIE), 1);
}

TEST_F(ReadoutTest, ErrVersion) {
  auto Res =
      ESSParser.validate((char *)&ErrVersion[0], ErrVersion.size(), DataType);
  ASSERT_EQ(Res, -Parser::EHEADER);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_VERSION), 1);
}

TEST_F(ReadoutTest, V0HeaderCount) {
  auto Res =
      ESSParser.validate((char *)&OkVersionV0[0], OkVersionV0.size(), DataType);
  ASSERT_EQ(Res, -Parser::OK);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_VERSION_V0), 1);
}

TEST_F(ReadoutTest, V1HeaderCount) {
  auto Res =
      ESSParser.validate((char *)&OkVersionV1[0], OkVersionV1.size(), DataType);
  ASSERT_EQ(Res, -Parser::OK);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_VERSION_V1), 1);
}

TEST_F(ReadoutTest, ErrMaxOutputQueue) {
  auto Res = ESSParser.validate((char *)&ErrMaxOutputQueue[0],
                            ErrMaxOutputQueue.size(), DataType);
  ASSERT_EQ(Res, -Parser::EHEADER);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_OUTPUT_QUEUE), 1);
}

TEST_F(ReadoutTest, OkV0Version) {
  unsigned int Errors{0};
  unsigned int MinSize{7};
  for (unsigned int Size = MinSize; Size < OkVersionV0.size(); Size++) {
    Errors++;
    auto Res = ESSParser.validate((char *)&OkVersionV0[0], Size, DataType);
    ASSERT_EQ(Res, -Parser::ESIZE);
    ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_SIZE), Errors);
    ASSERT_EQ(ESSParser.Packet.DataPtr, nullptr);
  }
  auto Res =
      ESSParser.validate((char *)&OkVersionV0[0], OkVersionV0.size(), DataType);
  ASSERT_EQ(Res, Parser::OK);
  ASSERT_EQ(ESSParser.Packet.DataPtr,
            (char *)(&OkVersionV0[0] + sizeof(Parser::PacketHeaderV0)));
  ASSERT_EQ(ESSParser.Packet.DataLength, 0);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_VERSION_V0), 1);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_VERSION_V1), 0);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_OUTPUT_QUEUE), 0);
}

TEST_F(ReadoutTest, OkV1Version) {
  unsigned int Errors{0};
  unsigned int MinSize{7};
  for (unsigned int Size = MinSize; Size < OkVersionV1.size(); Size++) {
    Errors++;
    auto Res = ESSParser.validate((char *)&OkVersionV1[0], Size, DataType);
    ASSERT_EQ(Res, -Parser::ESIZE);
    ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_SIZE), Errors);
    ASSERT_EQ(ESSParser.Packet.DataPtr, nullptr);
  }
  auto Res =
      ESSParser.validate((char *)&OkVersionV1[0], OkVersionV1.size(), DataType);
  ASSERT_EQ(Res, Parser::OK);
  ASSERT_EQ(ESSParser.Packet.DataPtr,
            (char *)(&OkVersionV1[0] + sizeof(Parser::PacketHeaderV1)));
  ASSERT_EQ(ESSParser.Packet.DataLength, 0);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_VERSION_V1), 1);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_VERSION_V0), 0);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_OUTPUT_QUEUE), 0);
}

TEST_F(ReadoutTest, SeqNumbers) {
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_SEQNO), 0);

  ESSParser.validate((char *)&OkVersionV0[0], OkVersionV0.size(), DataType);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_SEQNO), 1);

  ESSParser.validate((char *)&OkVersionNextSeq[0], OkVersionNextSeq.size(),
                 DataType);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_SEQNO), 1);

  ESSParser.validate((char *)&OkVersionNextSeq[0], OkVersionNextSeq.size(),
                 DataType);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_SEQNO), 2);
}

TEST_F(ReadoutTest, BadReadoutTypev0) {
  auto Res = ESSParser.validate((char *)&OkThreeLokiReadoutsV0[0],
                            OkThreeLokiReadoutsV0.size(), 0xff);
  ASSERT_EQ(Res, -Parser::EHEADER);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_TYPE), 1);
}

TEST_F(ReadoutTest, BadReadoutTypeV1) {
  auto Res = ESSParser.validate((char *)&OkThreeLokiReadoutsV1[0],
                            OkThreeLokiReadoutsV1.size(), 0xff);
  ASSERT_EQ(Res, -Parser::EHEADER);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_TYPE), 1);
}

TEST_F(ReadoutTest, DataLengthMismatchV0) {
  auto res = ESSParser.validate((char *)&OkThreeLokiReadoutsV0[0],
                            OkThreeLokiReadoutsV0.size() - 1, DataType);
  ASSERT_EQ(res, -Parser::ESIZE);
}

TEST_F(ReadoutTest, DataLengthMismatchV1) {
  auto res = ESSParser.validate((char *)&OkThreeLokiReadoutsV1[0],
                            OkThreeLokiReadoutsV1.size() - 1, DataType);
  ASSERT_EQ(res, -Parser::ESIZE);
}

TEST_F(ReadoutTest, PulseTimeFracErrorV0) {
  auto res = ESSParser.validate((char *)&ErrPulseTimeFracV0[0],
                            ErrPulseTimeFracV0.size(), DataType);
  ASSERT_EQ(res, -Parser::EHEADER);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_TIMEFRAC), 1);
}

TEST_F(ReadoutTest, PulseTimeFracErrorV1) {
  auto res = ESSParser.validate((char *)&ErrPulseTimeFracV1[0],
                            ErrPulseTimeFracV1.size(), DataType);
  ASSERT_EQ(res, -Parser::EHEADER);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_TIMEFRAC), 1);
}

TEST_F(ReadoutTest, PrevPulseTimeFracErrorV0) {
  auto res = ESSParser.validate((char *)&ErrPrevPulseTimeFracV0[0],
                            ErrPrevPulseTimeFracV0.size(), DataType);
  ASSERT_EQ(res, -Parser::EHEADER);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_TIMEFRAC), 1);
}

TEST_F(ReadoutTest, PrevPulseTimeFracErrorV1) {
  auto res = ESSParser.validate((char *)&ErrPrevPulseTimeFracV1[0],
                            ErrPrevPulseTimeFracV1.size(), DataType);
  ASSERT_EQ(res, -Parser::EHEADER);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_TIMEFRAC), 1);
}

TEST_F(ReadoutTest, MaxPulseTimeErrorV0) {
  auto res = ESSParser.validate((char *)&ErrMaxPulseTimeV0[0],
                            ErrMaxPulseTimeV0.size(), DataType);
  ASSERT_EQ(res, -Parser::EHEADER);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_TIMEHIGH), 1);
}

TEST_F(ReadoutTest, MaxPulseTimeErrorV1) {
  auto res = ESSParser.validate((char *)&ErrMaxPulseTimeV1[0],
                            ErrMaxPulseTimeV1.size(), DataType);
  ASSERT_EQ(res, -Parser::EHEADER);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_TIMEHIGH), 1);
}

TEST_F(ReadoutTest, NullBuffer) {
  auto Res = ESSParser.validate(nullptr, 100, DataType);
  ASSERT_EQ(Res, -Parser::EBUFFER);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_BUFFER), 1);
}

TEST_F(ReadoutTest, ZeroSizeBuffer) {
  auto Res = ESSParser.validate((char *)100, 0, DataType);
  ASSERT_EQ(Res, -Parser::EBUFFER);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_BUFFER), 1);
}

TEST_F(ReadoutTest, InvalidVersion) {
  char Buffer[6] = {0x00, 0x02, 0x53, 0x53, 0x45, 0x00};
  auto Res = ESSParser.validate(Buffer, sizeof(Buffer), DataType);
  ASSERT_EQ(Res, -Parser::EHEADER);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_VERSION), 1);
}

TEST_F(ReadoutTest, InvalidCookie) {
  char Buffer[6] = {0x00, 0x01, 0x53, 0x53, 0x44, 0x00};
  auto Res = ESSParser.validate(Buffer, sizeof(Buffer), DataType);
  ASSERT_EQ(Res, -Parser::EHEADER);
  ASSERT_EQ(Stats.valueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_COOKIE), 1);
}

} // namespace ESSReadout

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}