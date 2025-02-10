// Copyright (C) 2023 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for Timepix3Base
///
//===----------------------------------------------------------------------===//

#include <memory>
#include <modules/timepix3/Counters.h>
#include <string>

#include <common/debug/Trace.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <timepix3/Timepix3Base.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

// clang-format off

class Timepix3BaseTest : public ::testing::Test {
public:
  BaseSettings Settings;
  std::unique_ptr<Timepix3::Timepix3Base> Readout;
  Counters* testCounters;

  void SetUp() override {
    Settings.NoHwCheck = true;
    Settings.ConfigFile = TIMEPIX_CONFIG;
    Settings.DetectorName = "timepix3";
    Settings.UpdateIntervalSec = 0;
    Settings.DumpFilePrefix = "deleteme_";

    Readout = std::make_unique<Timepix3::Timepix3Base>(Settings);
    testCounters = &Readout->Counters;
  }

  void TearDown() override {}
};

TEST_F(Timepix3BaseTest, Constructor) {
  Timepix3::Timepix3Base Readout(Settings);
  EXPECT_EQ(Readout.ITCounters.RxPackets, 0);
}

std::vector<uint8_t> BadTestPacket{0x00, 0x01, 0x02};

/// | ESS Header    |
/// | Data header 1 | Readout 1 | Readout 2 | Readout 3 |
/// | Data header 2 |
/// | Data block 1  |
/// | Data header 1 |
/// | Data block 1  |
///
std::vector<uint8_t> TestPacket2{
    // ESS header
                0x00, 0x00, // pad, v0
    0x45, 0x53, 0x53, 0x30, //  'E' 'S' 'S' 0x00
    0xae, 0x00, 0x00, 0x00, // 0x96 = 150 bytes
    0x11, 0x00, 0x00, 0x00, // Pulse time High (17s)
    0x00, 0x01, 0x00, 0x00, // Pulse time Low (256 clocks)
    0x11, 0x00, 0x00, 0x00, // Prev PT
    0x00, 0x00, 0x00, 0x00, //
    0x01, 0x00, 0x00, 0x00, // Seq number 1


    // Data Header 1
    0x00, 0x00, 0x18, 0x00, // ring 0, fen 0, data size 64 bytes
    // Readout
    0x11, 0x00, 0x00, 0x00, // time high (17s)
    0x01, 0x01, 0x00, 0x00, // time low (257 clocks)
    0x00, 0x00, 0x00, 0x00, // fpga 0, tube 0
    0x01, 0x01, 0x02, 0x01, // amp a, amp b
    0x03, 0x01, 0x04, 0x01, // amp c, amp d


    // Data Header 2
    // Ring 5 is invalid -> RingErrors++
    0x07, 0x00, 0x18, 0x00, // ring 7, fen 0, data size 64 bytes
    // Readout
    0x11, 0x00, 0x00, 0x00, //time high 17s
    0x01, 0x02, 0x00, 0x00, // time low (257 clocks)
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x02, 0x01,
    0x03, 0x01, 0x04, 0x01,


    // Data Header 3
    // FEN 4 is invalid -> FENErrors++ (for loki only so far)
    0x01, 0x04, 0x18, 0x00, // ring 1, fen 4, size 24 bytes
    // Readout
    0x11, 0x00, 0x00, 0x00,
    0x02, 0x02, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x02, 0x02,
    0x03, 0x02, 0x04, 0x02,


    // Data Header 4 
    0x00, 0x00, 0x18, 0x00, // ring 0, fen 0, data size 64 bytes
    // Readout
    0x11, 0x00, 0x00, 0x00, // time high (17s)
    0x03, 0x01, 0x00, 0x00, // time low (259 clocks)
    0x00, 0x00, 0x00, 0x00, // amplitudes are all 0, PixelErrors ++
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,

    // Data Header 5
    0x00, 0x00, 0x18, 0x00, // ring 0, fen 0, data size 64 bytes
    // Readout
    0x12, 0x00, 0x00, 0x00, // time high (18s)
    0x01, 0x01, 0x00, 0x00, // time low (257 clocks)
    0x00, 0x00, 0x00, 0x00, // fpga 0, tube 0
    0x01, 0x01, 0x02, 0x01, // amp a, amp b
    0x03, 0x01, 0x04, 0x01, // amp c, amp d

    // Data Header 6
    0x00, 0x00, 0x18, 0x00, // ring 0, fen 0, data size 64 bytes
    // Readout
    0x0a, 0x00, 0x00, 0x00, // time high (10s)
    0x01, 0x01, 0x00, 0x00, // time low (257 clocks)
    0x00, 0x00, 0x00, 0x00, // fpga 0, tube 0
    0x01, 0x01, 0x02, 0x01, // amp a, amp b
    0x03, 0x01, 0x04, 0x01, // amp c, amp d
};
// clang-format on

/// \todo not a test yet, write correct pixel readout packet format
TEST_F(Timepix3BaseTest, DataReceive) {
  Timepix3::Timepix3Base Readout(Settings);

  writePacketToRxFIFO<Timepix3::Timepix3Base>(Readout, BadTestPacket);

  EXPECT_EQ(testCounters->PixelReadouts, 0);
  Readout.stopThreads();
}

TEST_F(Timepix3BaseTest, DataReceiveGood) {
  Timepix3::Timepix3Base Readout(Settings);

  writePacketToRxFIFO<Timepix3::Timepix3Base>(Readout, TestPacket2);

  /// \todo not a test yet, write correct pixel readout packet format
  // EXPECT_EQ(Readout.Counters.PixelReadouts, 6);
  Readout.stopThreads();
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
