// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for TTL Monitor Parser
///
//===----------------------------------------------------------------------===//

#include <common/readout/ess/Parser.h>
#include <common/testutils/TestBase.h>
#include <modules/ttlmonitor/geometry/Parser.h>

namespace TTLMonitor {

/// \brief Badsize in readout data field
std::vector<uint8_t> DataBadFracTime{
    // First readout
    0x16, 0x00, 0x10, 0x00, // Data header - Ring 22, FEN 0, Size 17
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x13, 0x93, 0x3f, 0x05, // Time LO 88.052.499 ok, max value
    0x00, 0x00, 0x00, 0x01, // ADC 0x100

    // Second readout
    0x16, 0x00, 0x10, 0x00, // Data header - Ring 22, FEN 0, Size 15
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x14, 0x93, 0x3f, 0x15, // Time LO 88.052.500 not ok
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
};

/// \brief Badsize in readout data field
std::vector<uint8_t> DataBadSize{
    // First readout
    0x16, 0x00, 0x11, 0x00, // Data header - Ring 22, FEN 0, Size 17
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x00, 0x00, 0x00, 0x01, // ADC 0x100

    // Second readout
    0x16, 0x00, 0x0f, 0x00, // Data header - Ring 22, FEN 0, Size 15
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
};

/// \brief Good Data
std::vector<uint8_t> DataGood{
    // First readout
    0x16, 0x00, 0x10, 0x00, // Data header - Ring 22, FEN 0, Size 16
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x00, 0x00, 0x00, 0x01, // ADC 0x100

    // Second readout
    0x16, 0x00, 0x10, 0x00, // Data header - Ring 22, FEN 0, Size 16
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
};

class TTLMonitorParserTest : public TestBase {
protected:
  ESSReadout::Parser::PacketDataV0 PacketData;
  Parser parser;
  void SetUp() override {
    PacketData.HeaderPtr = nullptr;
    PacketData.DataPtr = nullptr;
    PacketData.DataLength = 0;
    PacketData.Time.setReference(0, 0);
    PacketData.Time.setPrevReference(0, 0);
  }
  void TearDown() override {}

  void makeHeader(std::vector<uint8_t> &testdata) {
    PacketData.DataPtr = (char *)&testdata[0];
    PacketData.DataLength = testdata.size();
  }
};

TEST_F(TTLMonitorParserTest, Constructor) {
  ASSERT_EQ(parser.Stats.Readouts, 0);
}

// nullptr as buffer
TEST_F(TTLMonitorParserTest, ErrorBufferPtr) {
  PacketData.DataPtr = 0;
  PacketData.DataLength = 100;
  parser.parse(PacketData);
  ASSERT_EQ(parser.Stats.ErrorSize, 1);
  ASSERT_EQ(parser.Stats.Readouts, 0);
}

// no data in buffer
TEST_F(TTLMonitorParserTest, NoData) {
  makeHeader(DataGood);
  PacketData.DataLength = 0;
  parser.parse(PacketData);
  ASSERT_EQ(parser.Stats.Readouts, 0);
}

// invalid data size provided in call
TEST_F(TTLMonitorParserTest, ErrorHdrDataSize) {
  makeHeader(DataGood);
  PacketData.DataLength = 19;

  parser.parse(PacketData);
  ASSERT_EQ(parser.Stats.ErrorSize, 1);
  ASSERT_EQ(parser.Stats.Readouts, 0);
}

// invalid data length in readout
TEST_F(TTLMonitorParserTest, ErrorDataSize) {
  makeHeader(DataBadSize);

  parser.parse(PacketData);
  ASSERT_EQ(parser.Stats.ErrorDataLength, 2);
  ASSERT_EQ(parser.Stats.Readouts, 2);
}

// invalid fractional time
TEST_F(TTLMonitorParserTest, ErrorDataTime) {
  makeHeader(DataBadFracTime);

  parser.parse(PacketData);
  ASSERT_EQ(parser.Stats.ErrorTimeFrac, 1);
  ASSERT_EQ(parser.Stats.Readouts, 2);
}

TEST_F(TTLMonitorParserTest, DataGood) {
  makeHeader(DataGood);

  parser.parse(PacketData);
  ASSERT_EQ(parser.Stats.ErrorSize, 0);
  ASSERT_EQ(parser.Stats.Readouts, 2);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
} // namespace TTLMonitor
