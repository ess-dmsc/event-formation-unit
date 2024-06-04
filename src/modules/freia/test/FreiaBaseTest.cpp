// Copyright (C) 2021 - 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for FreiaBase
///
//===----------------------------------------------------------------------===//

#include <cinttypes>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <freia/FreiaBase.h>
#include <string>
#include <vector>

// clang-format off
std::vector<uint8_t> BadTestPacket{
                0x00, 0x00, // pad, v0
    0x45, 0x53, 0x53, 0x48, // 'E', 'S', 'S', type 0x48
    0x46, 0x00, 0x0B, 0x00, // len(0x005e), OQ11, TSrc0
    0x00, 0x00, 0x00, 0x00, // PT HI
    0x00, 0x00, 0x00, 0x00, // PT LO
    0x00, 0x00, 0x00, 0x00, // PPT HI
    0x00, 0x00, 0x00, 0x00, // PPT Lo
    0x08, 0x00, 0x00, 0x00, // Seq number 8

    // First readout
    0x01, 0x01, 0x14, 0x00, // Data Header
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
    0x00, 0x00, 0x00, 0x00, // GEO 0, TDC 0, VMM 0, CH 0

    // Second readout
    0x02, 0x02, 0x14, 0x00, // Data Header
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
    0x00, 0x00, 0x00, 0x00, // GEO 0, TDC 0, VMM 0, CH 0
};

//clang-format on

class FreiaBaseTest : public ::testing::Test {
public:
  BaseSettings Settings;

  void SetUp() override {
    Settings.ConfigFile = FREIA_FULL;
    Settings.KafkaTopic = "freia_detector";
    Settings.RxSocketBufferSize = 100000;
    Settings.NoHwCheck = true;
  }
  void TearDown() override {}
};


TEST_F(FreiaBaseTest, Constructor) {
  Freia::FreiaBase Readout(Settings);
  EXPECT_EQ(Readout.ITCounters.RxPackets, 0);
  EXPECT_EQ(Readout.Counters.VMMStats.Readouts, 0);
}

TEST_F(FreiaBaseTest, DataReceive) {
  Freia::FreiaBase Readout(Settings);

  writePacketToRxFIFO(Readout, BadTestPacket);

  // number of readouts in TestPacket
  EXPECT_EQ(Readout.Counters.VMMStats.Readouts, 2);
  EXPECT_EQ(Readout.Counters.VMMStats.DataReadouts, 2);
  // Freia has 32 cassettes, and 32 event builders, so match attempt
  // count increases by 32 for each packet received
  EXPECT_EQ(Readout.Counters.MatcherStats.MatchAttemptCount, 32);
  Readout.stopThreads();
}

TEST_F(FreiaBaseTest, DataReceiveBadHeader) {
  Freia::FreiaBase Readout(Settings);

  BadTestPacket[0] = 0xff; // pad should be 0
  writePacketToRxFIFO(Readout, BadTestPacket);

  EXPECT_EQ(Readout.Counters.ErrorESSHeaders, 1);
  EXPECT_EQ(Readout.Counters.VMMStats.Readouts, 0); // no readouts: bad header
  EXPECT_EQ(Readout.Counters.VMMStats.DataReadouts, 0);
  Readout.stopThreads();
}


TEST_F(FreiaBaseTest, EmulateFIFOError) {
  Freia::FreiaBase Readout(Settings);
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
