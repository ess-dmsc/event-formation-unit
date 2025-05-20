// Copyright (C) 2022 - 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for CbmBase
///
//===----------------------------------------------------------------------===//

#include <cbm/CbmBase.h>
#include <cinttypes>
#include <common/testutils/TestBase.h>
#include <filesystem>
#include <string>
#include <vector>

using std::filesystem::path;

// clang-format off
std::vector<uint8_t> BadTestPacket {
              0x00, 0x00, // pad, v0
  0x45, 0x53, 0x53, 0x10, // 'E', 'S', 'S', type 0x10
  0x46, 0x00, 0x0B, 0x00, // len(70, 0x0046), OQ11, TSrc0
  0x00, 0x00, 0x00, 0x00, // PT HI
  0x00, 0x00, 0x00, 0x00, // PT LO
  0x00, 0x00, 0x00, 0x00, // PPT HI
  0x00, 0x00, 0x00, 0x00, // PPT Lo
  0x08, 0x00, 0x00, 0x00, // Seq number 8

  // First monitor readout - Valid
  0x16, 0x00, 0x14, 0x00,  // Data Header: Fiber 22, FEN 0, Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  // Second monitor readout - invalid Ring
  0x18, 0x00, 0x14, 0x00,  // Data Header: Fiber 24, FEN 0, Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
};

std::vector<uint8_t> GoodTestPacket {
                0x00, 0x01, // pad, v1
    0x45, 0x53, 0x53, 0x10, // 'E', 'S', 'S', type 0x10 (CBM)
    0x5C, 0x00, 0x0B, 0x00, // len(92 0x005C), OQ11, TSrc0
    0x00, 0x00, 0x00, 0x00, // PT HI
    0x00, 0x00, 0x00, 0x00, // PT LO
    0x00, 0x00, 0x00, 0x00, // PPT HI
    0x00, 0x00, 0x00, 0x00, // PPT Lo
    0x00, 0x00, 0x00, 0x00, // Seq number 0
    0x00, 0x00,                       // CMAC Padding

  // First monitor readout - Valid TTL
  0x16, 0x00, 0x14, 0x00,  // Data Header: Fiber 22, FEN 0, Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x01, 0x00, 0x00, 0x00,  // Type 1, Ch 0, ADC 0
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  // Second monitor readout - Valid IBM
  0x16, 0x01, 0x14, 0x00,  // Data Header: Fiber 22, FEN 1, Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x03, 0x01, 0x01, 0x00,  // Type 3, Ch 1, ADC 1
  0xFF, 0x00, 0x00, 0x00,  // NPOS 255

  // Third monitor readout - Valid TTL
  0x18, 0x00, 0x14, 0x00,  // Data Header: Fiber 24, FEN 0, Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x01, 0x01, 0x01, 0x00,  // Type 1, Ch 1, ADC 1
  0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
};

// clang-format on

using namespace cbm;

class CbmBaseTest : public ::testing::Test {
public:
  BaseSettings Settings;
  inline static path FullConfigFile{""};

  void SetUp() override {

    // Get base test dir
    path TestDir = path(__FILE__).parent_path();
    // Define test files
    Settings.ConfigFile = TestDir / path("cbm_base_test.json");
    Settings.KafkaTopic = "freia_beam_monitor";
    Settings.NoHwCheck = true;
  }

  void TearDown() override {}
};

TEST_F(CbmBaseTest, Constructor) {
  CbmBase DetectorBase(Settings);
  EXPECT_EQ(DetectorBase.ITCounters.RxPackets, 0);
  EXPECT_EQ(DetectorBase.Counters.CbmStats.Readouts, 0);
}

TEST_F(CbmBaseTest, DataReceive) {
  CbmBase DetectorBase(Settings);

  writePacketToRxFIFO(DetectorBase, BadTestPacket);

  EXPECT_EQ(DetectorBase.Counters.CbmStats.Readouts, 2);
  DetectorBase.stopThreads();
}

TEST_F(CbmBaseTest, DataReceiveBadHeader) {
  CbmBase DetectorBase(Settings);

  BadTestPacket[0] = 0xff; // pad should be 0
  writePacketToRxFIFO(DetectorBase, BadTestPacket);

  EXPECT_EQ(DetectorBase.Counters.ErrorESSHeaders, 1);

  // no readouts as header is bad
  EXPECT_EQ(DetectorBase.Counters.CbmStats.Readouts, 0);
  DetectorBase.stopThreads();
}

TEST_F(CbmBaseTest, EmulateFIFOError) {
  CbmBase DetectorBase(Settings);
  EXPECT_EQ(DetectorBase.Counters.FifoSeqErrors, 0);

  DetectorBase.startThreads();

  unsigned int rxBufferIndex = DetectorBase.RxRingbuffer.getDataIndex();
  ASSERT_EQ(rxBufferIndex, 0);

  DetectorBase.RxRingbuffer.setDataLength(rxBufferIndex, 0); ///< invalid size

  ASSERT_TRUE(DetectorBase.InputFifo.push(rxBufferIndex));
  DetectorBase.RxRingbuffer.getNextBuffer();

  waitForProcessing(DetectorBase);

  EXPECT_EQ(DetectorBase.Counters.FifoSeqErrors, 1);
  DetectorBase.stopThreads();
}

TEST_F(CbmBaseTest, DataReceiveGoodPacket) {
  CbmBase DetectorBase(Settings);

  writePacketToRxFIFO(DetectorBase, GoodTestPacket);

  // Ensure timer is timedout
  std::this_thread::sleep_for(std::chrono::milliseconds(2500));

  EXPECT_EQ(DetectorBase.Counters.CbmStats.Readouts, 3);
  EXPECT_EQ(DetectorBase.Counters.Event0DReadoutsProcessed, 1);
  EXPECT_EQ(DetectorBase.Counters.IBMReadoutsProcessed, 1);
  EXPECT_EQ(DetectorBase.Counters.ProduceCauseTimeout, 2);
  DetectorBase.stopThreads();
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
