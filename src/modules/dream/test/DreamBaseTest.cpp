// Copyright (C) 2021 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief test for class DreamBase
///
//===----------------------------------------------------------------------===//

#include <string>

#include <common/readout/ess/Parser.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <dream/DreamBase.h>


std::string dreamjson = R"(
  {
    "Detector" : "DREAM",

    "MaxPulseTimeDiffNS" : 357142855
  },

  "Config" : [
    { "Ring" :  0, "FEN":  0, "Type": "BwEndCap"}
  ]
)";


class DreamBaseTest : public ::testing::Test {
public:
  BaseSettings Settings;

  void SetUp() override {
    Settings.UpdateIntervalSec = 0;
    Settings.RxSocketBufferSize = 100000;
    Settings.NoHwCheck = true;
    Settings.ConfigFile = "deleteme_dream.json";
    Settings.DetectorName = "dream";
    Settings.GraphitePrefix = "dream";
    Settings.GraphiteRegion = "test";
  }

  void TearDown() override {}
};

TEST_F(DreamBaseTest, Constructor) {
  Dream::DreamBase Readout(Settings, DetectorType::DREAM);
  ASSERT_EQ(Readout.getStatPrefix(1), "dream.test.");
  EXPECT_EQ(Readout.getInputCounters().RxPackets, 0);
}

/// | ESS Header    |
/// | Data header 1 | Readout 1 | Readout 2 | Readout 3 |
/// | Data header 2 |
/// | Data block 1  |
/// | Data header 1 |
/// | Data block 1  |
///
// clang-format off
std::vector<uint8_t> TestPacket2{
    // ESS header
    0x00, 0x00,             // pad, v0
    0x45, 0x53, 0x53, 0x60, //  'E' 'S' 'S' 0x60
    0x2e, 0x00, 0x00, 0x00, // 0x002e - 46 bytes
    0x11, 0x00, 0x00, 0x00, // Pulse time High (17s)
    0x00, 0x01, 0x00, 0x00, // Pulse time Low (256 clocks)
    0x11, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, // Seq number 1

    // Data Header 1
    0x00, 0x01, 0x10, 0x00, // ring 0, fen 1, data size 16 bytes

    // Readout 1
    0x22, 0x00, 0x00, 0x00, // tof 34 (0x22)
    0x00, 0x00, 0x14, 0x05, // unused 00 00 module 20, sumo 5
    0x00, 0x00, 0xCC, 0xAA  // normal operation, cathode 0xcc, anode 0xaa
};

std::vector<uint8_t> TestPacket3{
    // ESS header
    0x00, 0x00,             // pad, v0
    0x45, 0x53, 0x53, 0x61, //  'E' 'S' 'S' 0x61 - not DREAM
    0x2e, 0x00, 0x00, 0x00, // 0x002e - 46 bytes
    0x11, 0x00, 0x00, 0x00, // Pulse time High (17s)
    0x00, 0x01, 0x00, 0x00, // Pulse time Low (256 clocks)
    0x11, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, // Seq number 1

    // Data Header 1
    0x00, 0x01, 0x10, 0x00, // ring 0, fen 1, data size 16 bytes

    // Readout 1
    0x22, 0x00, 0x00, 0x00, // tof 34 (0x22)
    0x00, 0x00, 0x14, 0x05, // unused 00 00 module 20, sumo 5
    0x00, 0x00, 0xCC, 0xAA  // normal operation, cathode 0xcc, anode 0xaa
};
// clang-format off

TEST_F(DreamBaseTest, DataReceiveGood) {
  Dream::DreamBase Readout(Settings, DetectorType::DREAM);

  writePacketToRxFIFO(Readout, TestPacket2);

  EXPECT_EQ(Readout.Counters.Readouts, 1);
  EXPECT_EQ(Readout.Counters.DataHeaders, 1);
  Readout.stopThreads();
}

TEST_F(DreamBaseTest, DataReceiveBad) {
  Dream::DreamBase Readout(Settings, DetectorType::DREAM);

  writePacketToRxFIFO(Readout, TestPacket3);

  EXPECT_EQ(Readout.Counters.Readouts, 0);
  EXPECT_EQ(Readout.Counters.DataHeaders, 0);
  EXPECT_EQ(Readout.Counters.ErrorESSHeaders, 1);
  Readout.stopThreads();
}


TEST_F(DreamBaseTest, EmulateFIFOError) {
  Dream::DreamBase Readout(Settings, DetectorType::DREAM);
  EXPECT_EQ(Readout.getInputCounters().FifoSeqErrors, 0);

  Readout.startThreads();

  unsigned int rxBufferIndex = Readout.RxRingbuffer.getDataIndex();
  ASSERT_EQ(rxBufferIndex, 0);

  Readout.RxRingbuffer.setDataLength(rxBufferIndex, 0); ///< invalid size

  ASSERT_TRUE(Readout.InputFifo.push(rxBufferIndex));
  Readout.RxRingbuffer.getNextBuffer();

  waitForProcessing(Readout);

  EXPECT_EQ(Readout.getInputCounters().FifoSeqErrors, 1);
  Readout.stopThreads();
}

int main(int argc, char **argv) {
  std::string filename{"deleteme_dream.json"};
  saveBuffer(filename, (void *)dreamjson.c_str(), dreamjson.size());

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
