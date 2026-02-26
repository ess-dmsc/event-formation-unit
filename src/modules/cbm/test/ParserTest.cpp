// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for Beam Monitor Parser
///
//===----------------------------------------------------------------------===//

#include <common/Statistics.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/TestBase.h>
#include <modules/cbm/readout/Parser.h>

using namespace cbm;

/// \brief
//clang-format off
std::vector<uint8_t> DataBadFracTime {

  // First readout
  0x16, 0x00, 0x14, 0x00,  // Data header - Fiber 22, FEN 0, Size 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x13, 0x93, 0x3f, 0x05,  // Time LO 88.052.499 ok, max value
  0x01, 0x00, 0x00, 0x01,  // Type 0x01, Channel 0, ADC 0x100
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  // Second readout
  0x16, 0x00, 0x14, 0x00,  // Data header - Fiber 22, FEN 0, Size 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x14, 0x93, 0x3f, 0x15,  // Time LO 88.052.500 not ok
  0x01, 0x00, 0x00, 0x01,  // Type 0x01, Channel 0, ADC 0x100
  0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
};

/// \brief Badsize in readout data field
std::vector<uint8_t> DataBadSize {

  // First readout
  0x16, 0x00, 0x15, 0x00,  // Data header - Ring 22, FEN 0, Size 21
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x01, 0x00, 0x00, 0x01,  // Type 0x01, Channel 0, ADC 0x100
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  // Second readout
  0x16, 0x00, 0x13, 0x00,  // Data header - Ring 22, FEN 0, Size 19
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x01, 0x00, 0x00, 0x01,  // Type 0x01, Channel 0, ADC 0x100
  0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
};

/// \brief Bad data type in readout data field
std::vector<uint8_t> DataBadType {

  // First readout - type low edge
  0x16, 0x00, 0x14, 0x00,  // Data header - Ring 22, FEN 0, Size 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // Type 0x00 (not good), Channel 0, ADC 0x100
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  // Second readout - type high edge
  0x16, 0x00, 0x14, 0x00,  // Data header - Ring 22, FEN 0, Size 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x04, 0x00, 0x00, 0x01,  // Type 0x04 (not good), Channel 0, ADC 0x100
  0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
};

/// \brief Good Data
std::vector<uint8_t> DataGood {

  // First readout
  0x16, 0x00, 0x14, 0x00,  // Data header - Ring 22, FEN 0, Size 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x01, 0x00, 0x00, 0x01,  // Type 0x01, Channel 0, ADC 0x000
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  // Second readout
  0x16, 0x00, 0x14, 0x00,  // Data header - Ring 22, FEN 0, Size 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x03, 0x00, 0x00, 0x01,  // Type 0x01, Channel 0, ADC 0x100
  0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
};

/// \brief Bad FEN value
std::vector<uint8_t> BadFENData {

  0x17, 0x22, 0x14, 0x00, // Data Header - Fiber 23, FEN 34, Size 20
  0x00, 0x00, 0x00, 0x00, // Time HI 0 s
  0x11, 0x00, 0x00, 0x00, // Time LO 17 ticks
  0x01, 0x00, 0x01, 0x00, // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00  // XPos 0, YPos 0
};

/// \brief Bad ADC (NPOD) value for IBM readout
std::vector<uint8_t> BadIBMNPOSData {

  // First readout
  0x16, 0x00, 0x14, 0x00,  // Data header - Ring 22, FEN 0, Size 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x03, 0x00, 0x00, 0x00,  // Type 0x01, Channel 0, ADC 0x000
  0xFF, 0xFF, 0xFF, 0x00,  // NPOS 0x00FFFFFF (Good)

  // Second readout
  0x16, 0x00, 0x14, 0x00,  // Data header - Ring 22, FEN 0, Size 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x03, 0x00, 0x00, 0x01,  // Type 0x03, Channel 0, ADC 0x100
  0xFF, 0xFF, 0xFF, 0x01   // NPOS 0x01FFFFFF (Bad)
};

/// \brief Good ADC values for event and non event readouts
std::vector<uint8_t> GoodNonIBMADCData{
    // First readout
    0x16, 0x00, 0x14, 0x00, // Data header - Ring 22, FEN 0, Size 20
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x01, 0x00, 0x01, 0x00, // Type 0x01, Channel 0, ADC 0x100
    0x00, 0x00, 0x00, 0x00, // XPos 0, YPos 0

    // Second readout
    0x16, 0x00, 0x14, 0x00,  // Data header - Ring 22, FEN 0, Size 20
    0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
    0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
    0x01, 0x00, 0x00, 0x00,  // Type 0x01, Channel 0, ADC 0x000
    0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
};

/// \brief Good ADC values for event and non event readouts
std::vector<uint8_t> Good2DData{
    // First readout
    0x16, 0x00, 0x14, 0x00,  // Data header - Ring 22, FEN 0, Size 20
    0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
    0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
    0x02, 0x00, 0x01, 0x00,  // Type 0x02, Channel 0, ADC 0x100
    0xFF, 0x00, 0xFF, 0x01,  // XPos 256, YPos 511

    // Second readout
    0x16, 0x00, 0x14, 0x00,  // Data header - Ring 22, FEN 0, Size 20
    0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
    0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
    0x02, 0x00, 0x00, 0x00,  // Type 0x02, Channel 0, ADC 0x000
    0xFF, 0x00, 0xFF, 0x01   // XPos 256, YPos 511
};

/// \brief Check data for SADC value read and convert
std::vector<uint8_t> SADCTestData{
    // First readout
    0x16, 0x00, 0x14, 0x00,  // Data header - Ring 22, FEN 0, Size 20
    0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
    0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
    0x02, 0x00, 0x01, 0x00,  // Type 0x02, Channel 0, ADC 0x100
    0xFF, 0xFF, 0xFF, 0x00,  // SAD 0x00FFFFFF -> ADC = 16777215, Count = 0

    // Second readout
    0x16, 0x00, 0x14, 0x00,  // Data header - Ring 22, FEN 0, Size 20
    0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
    0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
    0x02, 0x00, 0x00, 0x00,  // Type 0x02, Channel 0, ADC 0x000
    0x10, 0x01, 0xA3, 0x4A,  // SAD 0x4AA30110 -> ADC = 10682640, Count = 74
};

//clang-format on
class CbmParserTest : public TestBase {
protected:
  Statistics Stats;
  ESSReadout::Parser::PacketDataV0 PacketData{Stats};
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

TEST_F(CbmParserTest, Constructor) { ASSERT_EQ(parser.Stats.Readouts, 0); }

// nullptr as buffer
TEST_F(CbmParserTest, ErrorBufferPtr) {
  PacketData.DataPtr = 0;
  PacketData.DataLength = 100;
  parser.parse(PacketData);
  EXPECT_EQ(parser.Stats.ErrorSize, 1);
  EXPECT_EQ(parser.Stats.Readouts, 0);
}

// no data in buffer
TEST_F(CbmParserTest, NoData) {
  makeHeader(DataGood);
  PacketData.DataLength = 0;
  parser.parse(PacketData);
  EXPECT_EQ(parser.Stats.Readouts, 0);
}

// invalid data size provided in call
TEST_F(CbmParserTest, ErrorHdrDataSize) {
  makeHeader(DataGood);
  PacketData.DataLength = 19;

  parser.parse(PacketData);
  EXPECT_EQ(parser.Stats.ErrorSize, 1);
  EXPECT_EQ(parser.Stats.Readouts, 0);
  EXPECT_EQ(parser.Stats.ErrorType, 0);
}

// invalid data length in readout
TEST_F(CbmParserTest, ErrorDataSize) {
  makeHeader(DataBadSize);

  parser.parse(PacketData);
  EXPECT_EQ(parser.Stats.ErrorDataLength, 2);
  EXPECT_EQ(parser.Stats.Readouts, 2);
  EXPECT_EQ(parser.Stats.ErrorType, 0);
}

// invalid fractional time
TEST_F(CbmParserTest, ErrorDataTime) {
  makeHeader(DataBadFracTime);

  parser.parse(PacketData);
  EXPECT_EQ(parser.Stats.ErrorTimeFrac, 1);
  EXPECT_EQ(parser.Stats.Readouts, 2);
  EXPECT_EQ(parser.Stats.ErrorType, 0);
  EXPECT_EQ(parser.Stats.Readouts0D, 1);
  EXPECT_EQ(parser.Stats.Readouts2D, 0);
  EXPECT_EQ(parser.Stats.ReadoutsIBM, 0);
}

// invalid fractional time
TEST_F(CbmParserTest, ErrorDataType) {
  makeHeader(DataBadType);

  parser.parse(PacketData);
  EXPECT_EQ(parser.Stats.ErrorType, 2);
  EXPECT_EQ(parser.Stats.Readouts, 2);
  EXPECT_EQ(parser.Stats.Readouts0D, 0);
  EXPECT_EQ(parser.Stats.Readouts2D, 0);
  EXPECT_EQ(parser.Stats.ReadoutsIBM, 0);
}

TEST_F(CbmParserTest, DataGood) {
  makeHeader(DataGood);

  parser.parse(PacketData);
  EXPECT_EQ(parser.Stats.ErrorSize, 0);
  EXPECT_EQ(parser.Stats.Readouts, 2);
  EXPECT_EQ(parser.Stats.ErrorType, 0);
  EXPECT_EQ(parser.Stats.Readouts0D, 1);
  EXPECT_EQ(parser.Stats.Readouts2D, 0);
  EXPECT_EQ(parser.Stats.ReadoutsIBM, 1);
}

TEST_F(CbmParserTest, ErrorFENId) {
  makeHeader(BadFENData);

  parser.parse(PacketData);
  EXPECT_EQ(parser.Stats.Readouts, 1);
  EXPECT_EQ(parser.Stats.ErrorFEN, 1);
  EXPECT_EQ(parser.Stats.ErrorType, 0);
}

TEST_F(CbmParserTest, CheckADCValues) {
  makeHeader(BadIBMNPOSData);

  parser.parse(PacketData);
  EXPECT_EQ(parser.Stats.ErrorADC, 0);
  EXPECT_EQ(parser.Stats.Readouts, 2);
  EXPECT_EQ(parser.Stats.ErrorType, 0);
  EXPECT_EQ(parser.Stats.Readouts0D, 0);
  EXPECT_EQ(parser.Stats.Readouts2D, 0);
  EXPECT_EQ(parser.Stats.ReadoutsIBM, 2);

  makeHeader(GoodNonIBMADCData);

  parser.parse(PacketData);
  EXPECT_EQ(parser.Stats.ErrorADC, 0);
  EXPECT_EQ(parser.Stats.Readouts, 4);
  EXPECT_EQ(parser.Stats.ErrorType, 0);
  EXPECT_EQ(parser.Stats.Readouts0D, 2);
  EXPECT_EQ(parser.Stats.Readouts2D, 0);
  EXPECT_EQ(parser.Stats.ReadoutsIBM, 2);
}

TEST_F(CbmParserTest, Check2DValues) {
  makeHeader(Good2DData);

  parser.parse(PacketData);
  EXPECT_EQ(parser.Stats.ErrorADC, 0);
  EXPECT_EQ(parser.Stats.Readouts, 2);
  EXPECT_EQ(parser.Stats.ErrorType, 0);
  EXPECT_EQ(parser.Stats.Readouts0D, 0);
  EXPECT_EQ(parser.Stats.Readouts2D, 2);
  EXPECT_EQ(parser.Stats.ReadoutsIBM, 0);

  // Wrong geometry data will be checked on cbminstrument.
}

// Test that SADC data structure endian decoding works correct
TEST_F(CbmParserTest, CheckSDACStructure) {
  makeHeader(SADCTestData);

  parser.parse(PacketData);
  EXPECT_EQ(parser.Stats.ErrorADC, 0);
  EXPECT_EQ(parser.Stats.Readouts, 2);
  EXPECT_EQ(parser.Stats.ErrorType, 0);

  //Check SDAC
  EXPECT_EQ(parser.Result.size(), 2);

  Parser::NormADC &data = parser.Result.at(0).NADC;
  EXPECT_EQ(data.getNADC(), 16777215);
  EXPECT_EQ(data.MCASum, 0);

  data = parser.Result.at(1).NADC;
  EXPECT_EQ(data.getNADC(), 10682640);
  EXPECT_EQ(data.MCASum, 74);

  //Make write read test of ADC()
  uint32_t overFlow = 16777500;
  data.setNADC(overFlow);
  EXPECT_EQ(data.getNADC(), 0x00FFFFFF);

  overFlow -= 1000;
  data.setNADC(overFlow);
  EXPECT_EQ(data.getNADC(), overFlow);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
