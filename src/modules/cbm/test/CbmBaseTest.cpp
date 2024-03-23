// Copyright (C) 2022 - 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for CbmBase
///
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <cinttypes>
#include <string>
#include <cbm/CbmBase.h>
#include <vector>

// clang-format off
std::vector<uint8_t> TestPacket {
              0x00, 0x00, // pad, v0
  0x45, 0x53, 0x53, 0x10, // 'E', 'S', 'S', type 0x10
  0x46, 0x00, 0x0B, 0x00, // len(0x0046), OQ11, TSrc0
  0x00, 0x00, 0x00, 0x00, // PT HI
  0x00, 0x00, 0x00, 0x00, // PT LO
  0x00, 0x00, 0x00, 0x00, // PPT HI
  0x00, 0x00, 0x00, 0x00, // PPT Lo
  0x08, 0x00, 0x00, 0x00, // Seq number 8

  // First monitor readout - Valid
  0x16, 0x00, 0x14, 0x00,  // Data Header - Fiber 22, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  // Second monitor readout - invalid Ring
  0x18, 0x00, 0x14, 0x00,  // Data Header, Fiber 24, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
};


// clang-format on

using namespace cbm;

class CbmBaseTest : public ::testing::Test {
public:
  BaseSettings Settings;

  void SetUp() override {
    Settings.ConfigFile = CBM_CONFIG;
    Settings.KafkaTopic = "freia_beam_monitor";
    Settings.NoHwCheck = true;
  }

  void TearDown() override {}
};

TEST_F(CbmBaseTest, Constructor) {
  CbmBase Readout(Settings);
  EXPECT_EQ(Readout.ITCounters.RxPackets, 0);
  EXPECT_EQ(Readout.Counters.CbmStats.Readouts, 0);
}

TEST_F(CbmBaseTest, DataReceive) {
  CbmBase Readout(Settings);

  writePacketToRxFIFO(Readout, TestPacket);

  EXPECT_EQ(Readout.Counters.CbmStats.Readouts, 2);
  Readout.stopThreads();
}

TEST_F(CbmBaseTest, DataReceiveBadHeader) {
  CbmBase Readout(Settings);

  TestPacket[0] = 0xff; // pad should be 0
  writePacketToRxFIFO(Readout, TestPacket);

  EXPECT_EQ(Readout.Counters.ErrorESSHeaders, 1);

  // no readouts as header is bad
  EXPECT_EQ(Readout.Counters.CbmStats.Readouts, 0);
  Readout.stopThreads();
}


TEST_F(CbmBaseTest, EmulateFIFOError) {
  CbmBase Readout(Settings);
  EXPECT_EQ(Readout.Counters.FifoSeqErrors, 0);

  Readout.startThreads();

  unsigned int rxBufferIndex = Readout.RxRingbuffer.getDataIndex();
  ASSERT_EQ(rxBufferIndex, 0);

  Readout.RxRingbuffer.setDataLength(rxBufferIndex, 0); ///< invalid size

  ASSERT_TRUE(Readout.InputFifo.push(rxBufferIndex));
  Readout.RxRingbuffer.getNextBuffer();

  waitForProcessing(Readout);

  EXPECT_EQ(Readout.Counters.FifoSeqErrors, 1);
  Readout.stopThreads();
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
