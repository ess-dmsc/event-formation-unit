// Copyright (C) 2022 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for TTL Monitor Parser
///
//===----------------------------------------------------------------------===//

#include <common/readout/ess/Parser.h>
#include <common/testutils/TestBase.h>
#include <modules/cbm/geometry/Parser.h>

using namespace cbm;

/// \brief
std::vector<uint8_t> DataBadFracTime{
    // First readout
    0x16, 0x00, 0x14, 0x00, // Data header - Fiber 22, FEN 0, Size 20
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x13, 0x93, 0x3f, 0x05, // Time LO 88.052.499 ok, max value
    0x01, 0x00, 0x00, 0x01, // Type 0x01, Channel 0, ADC 0x100
    0x00, 0x00, 0x00, 0x00,

    // Second readout
    0x16, 0x00, 0x14, 0x00, // Data header - Fiber 22, FEN 0, Size 20
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x14, 0x93, 0x3f, 0x15, // Time LO 88.052.500 not ok
    0x01, 0x00, 0x00, 0x01, // Type 0x01, Channel 0, ADC 0x100
    0x00, 0x00, 0x00, 0x00
};

/// \brief Badsize in readout data field
std::vector<uint8_t> DataBadSize{
    // First readout
    0x16, 0x00, 0x15, 0x00, // Data header - Ring 22, FEN 0, Size 21
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x01, 0x00, 0x00, 0x01, // Type 0x01, Channel 0, ADC 0x100
    0x00, 0x00, 0x00, 0x00,

    // Second readout
    0x16, 0x00, 0x13, 0x00, // Data header - Ring 22, FEN 0, Size 19
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x01, 0x00, 0x00, 0x01, // Type 0x01, Channel 0, ADC 0x100
    0x00, 0x00, 0x00, 0x00
};

/// \brief Bad data type in readout data field
std::vector<uint8_t> DataBadType{
    // First readout
    0x16, 0x00, 0x15, 0x00, // Data header - Ring 22, FEN 0, Size 21
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x00, 0x00, 0x00, 0x01, // Type 0x00 (not good), Channel 0, ADC 0x100
    0x00, 0x00, 0x00, 0x00,

    // Second readout
    0x16, 0x00, 0x13, 0x00, // Data header - Ring 22, FEN 0, Size 19
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x08, 0x00, 0x00, 0x01, // Type 0x08 (not good), Channel 0, ADC 0x100
    0x00, 0x00, 0x00, 0x00
};

/// \brief Good Data
std::vector<uint8_t> DataGood{
    // First readout
    0x16, 0x00, 0x14, 0x00, // Data header - Ring 22, FEN 0, Size 20
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x01, 0x00, 0x00, 0x01, // Type 0x01, Channel 0, ADC 0x100
    0x00, 0x00, 0x00, 0x00,

    // Second readout
    0x16, 0x00, 0x14, 0x00, // Data header - Ring 22, FEN 0, Size 20
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x01, 0x00, 0x00, 0x01, // Type 0x01, Channel 0, ADC 0x100
    0x00, 0x00, 0x00, 0x00
};

/// \brief Bad ADC values
std::vector<uint8_t> BadADCData{
    // First readout
    0x16, 0x00, 0x14, 0x00, // Data header - Ring 22, FEN 0, Size 20
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x03, 0x00, 0x00, 0x01, // Type 0x01, Channel 0, ADC 0x100
    0x00, 0x00, 0x00, 0x00,

    // Second readout
    0x16, 0x00, 0x14, 0x00, // Data header - Ring 22, FEN 0, Size 20
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x03, 0x00, 0x00, 0x01, // Type 0x01, Channel 0, ADC 0x100
    0x00, 0x00, 0x00, 0x00
};

/// \brief Good ADC values
std::vector<uint8_t> GoodADCData{
    // First readout
    0x16, 0x00, 0x14, 0x00, // Data header - Ring 22, FEN 0, Size 20
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x01, 0x00, 0x00, 0x01, // Type 0x01, Channel 0, ADC 0x100
    0x00, 0x00, 0x00, 0x00,

    // Second readout
    0x16, 0x00, 0x14, 0x00, // Data header - Ring 22, FEN 0, Size 20
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x03, 0x00, 0x00, 0x00, // Type 0x01, Channel 0, ADC 0x100
    0x00, 0x00, 0x00, 0x00
};

class CbmParserTest : public TestBase {
protected:
  ESSReadout::Parser::PacketDataV0 PacketData;
  Parser parser;
  void SetUp() override {
    PacketData.DataPtr = nullptr;
    PacketData.DataLength = 0;
    PacketData.Time.setReference(ESSReadout::ESSTime(0, 0));
    PacketData.Time.setPrevReference(ESSReadout::ESSTime(0, 0));
  }
  void TearDown() override {}

  void makeHeader(std::vector<uint8_t> &testdata) {
    PacketData.DataPtr = (char *)&testdata[0];
    PacketData.DataLength = testdata.size();
  }
};

TEST_F(CbmParserTest, Constructor) {
  ASSERT_EQ(parser.Stats.Readouts, 0);
}

// nullptr as buffer
TEST_F(CbmParserTest, ErrorBufferPtr) {
  PacketData.DataPtr = 0;
  PacketData.DataLength = 100;
  parser.parse(PacketData);
  ASSERT_EQ(parser.Stats.ErrorSize, 1);
  ASSERT_EQ(parser.Stats.Readouts, 0);
}

// no data in buffer
TEST_F(CbmParserTest, NoData) {
  makeHeader(DataGood);
  PacketData.DataLength = 0;
  parser.parse(PacketData);
  ASSERT_EQ(parser.Stats.Readouts, 0);
}

// invalid data size provided in call
TEST_F(CbmParserTest, ErrorHdrDataSize) {
  makeHeader(DataGood);
  PacketData.DataLength = 19;

  parser.parse(PacketData);
  ASSERT_EQ(parser.Stats.ErrorSize, 1);
  ASSERT_EQ(parser.Stats.Readouts, 0);
}

// invalid data length in readout
TEST_F(CbmParserTest, ErrorDataSize) {
  makeHeader(DataBadSize);

  parser.parse(PacketData);
  ASSERT_EQ(parser.Stats.ErrorDataLength, 2);
  ASSERT_EQ(parser.Stats.Readouts, 2);
}

// invalid fractional time
TEST_F(CbmParserTest, ErrorDataTime) {
  makeHeader(DataBadFracTime);

  parser.parse(PacketData);
  ASSERT_EQ(parser.Stats.ErrorTimeFrac, 1);
  ASSERT_EQ(parser.Stats.Readouts, 2);
}

// invalid fractional time
TEST_F(CbmParserTest, ErrorDataType) {
  makeHeader(DataBadType);

  parser.parse(PacketData);
  ASSERT_EQ(parser.Stats.ErrorType, 2);
  ASSERT_EQ(parser.Stats.Readouts, 2);
}

TEST_F(CbmParserTest, DataGood) {
  makeHeader(DataGood);

  parser.parse(PacketData);
  ASSERT_EQ(parser.Stats.ErrorSize, 0);
  ASSERT_EQ(parser.Stats.Readouts, 2);
}

TEST_F(CbmParserTest, CheckADCValues) {
  makeHeader(BadADCData);

  parser.parse(PacketData);
  ASSERT_EQ(parser.Stats.ErrorADC, 2);
  ASSERT_EQ(parser.Stats.Readouts, 2);

  makeHeader(GoodADCData);

  parser.parse(PacketData);
  ASSERT_EQ(parser.Stats.ErrorADC, 2);
  ASSERT_EQ(parser.Stats.Readouts, 4);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
