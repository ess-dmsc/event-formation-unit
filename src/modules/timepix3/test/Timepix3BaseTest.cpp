// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for Timepix3Base
///
//===----------------------------------------------------------------------===//

#include <string>

#include <timepix3/Timepix3Base.h>
#include <common/debug/Trace.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <common/testutils/TestUDPServer.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

/// Test configuration - two rings used (0 and 1)
/// TubesN = 8 and TubesZ = 4 implies four tube groups and
/// four FENs per ring. FENs are enumerated 0 - 3 and
/// Tube groups 0 - 4
// clang-format off
std::string configjson = R"(
{
  "Detector" : "timepix3",
  "XResolution" : 256,
  "YResolution" : 256
}
)";

class Timepix3BaseStandIn : public Timepix3::Timepix3Base {
public:
  Timepix3BaseStandIn(BaseSettings Settings)
      : Timepix3::Timepix3Base(Settings){};
  ~Timepix3BaseStandIn() = default;
  using Detector::Threads;
  using Timepix3::Timepix3Base::Counters;
};

class Timepix3BaseTest : public ::testing::Test {
public:
  void SetUp() override {
    Settings.RxSocketBufferSize = 100000;
    Settings.NoHwCheck = true;
    Settings.ConfigFile = "deleteme_timepix.json";
  }
  void TearDown() override {}

  std::chrono::duration<std::int64_t, std::milli> SleepTime{400};
  BaseSettings Settings;
};

TEST_F(Timepix3BaseTest, Constructor) {
  Timepix3BaseStandIn Readout(Settings);
  EXPECT_EQ(Readout.ITCounters.RxPackets, 0);
}



std::vector<uint8_t> TestPacket{0x00, 0x01, 0x02};

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

TEST_F(Timepix3BaseTest, DataReceive) {
  Settings.DetectorPort = 9002;
  Settings.DetectorName = "timepix3";
  Timepix3BaseStandIn Readout(Settings);
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort,
                       (unsigned char *)&TestPacket[0], TestPacket.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.ITCounters.RxPackets, 1);
  EXPECT_EQ(Readout.ITCounters.RxBytes, TestPacket.size());
  EXPECT_EQ(Readout.Counters.PixelReadouts, 0);
}


TEST_F(Timepix3BaseTest, DataReceiveGood) {
  XTRACE(DATA, DEB, "Running DataReceiveGood test");
  Settings.DetectorName = "timepix3";
  Settings.DetectorPort = 9002;
  Settings.UpdateIntervalSec = 0;
  Settings.DumpFilePrefix = "deleteme_";
  Timepix3BaseStandIn Readout(Settings);
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43127, Settings.DetectorPort,
                       (unsigned char *)&TestPacket2[0], TestPacket2.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.ITCounters.RxPackets, 1);
  EXPECT_EQ(Readout.ITCounters.RxBytes, TestPacket2.size());
  // todo, write correct pixel readout packet format
  // EXPECT_EQ(Readout.Counters.PixelReadouts, 6);
}

int main(int argc, char **argv) {
  std::string configfilename{"deleteme_timepix.json"};
  saveBuffer(configfilename, (void *)configjson.c_str(), configjson.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(configfilename);
  return RetVal;
}
