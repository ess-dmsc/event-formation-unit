// Copyright (C) 2021 European Spallation Source, see LICENSE file
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
#include <common/testutils/TestUDPServer.h>
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

class DreamBaseStandIn : public Dream::DreamBase {
public:
  DreamBaseStandIn(BaseSettings Settings) : Dream::DreamBase(Settings){};
  ~DreamBaseStandIn() = default;
  using Detector::Threads;
  using Dream::DreamBase::Counters;
};

class DreamBaseTest : public ::testing::Test {
public:
  void SetUp() override {
    Settings.RxSocketBufferSize = 100000;
    Settings.NoHwCheck = true;
    Settings.ConfigFile = "deleteme_dream.json";
  }
  void TearDown() override {}

  std::chrono::duration<std::int64_t, std::milli> SleepTime{400};
  BaseSettings Settings;
};

TEST_F(DreamBaseTest, Constructor) {
  DreamBaseStandIn Readout(Settings);
  EXPECT_EQ(Readout.Counters.RxPackets, 0);
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
  Settings.DetectorPort = 9001;
  Settings.UpdateIntervalSec = 0;
  DreamBaseStandIn Readout(Settings);
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43127, Settings.DetectorPort,
                       (unsigned char *)&TestPacket2[0], TestPacket2.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.Counters.RxPackets, 1);
  EXPECT_EQ(Readout.Counters.RxBytes, TestPacket2.size());
  EXPECT_EQ(Readout.Counters.Readouts, 1);
  EXPECT_EQ(Readout.Counters.DataHeaders, 1);
  EXPECT_EQ(Readout.Counters.GeometryErrors, 0);
  EXPECT_EQ(Readout.Counters.MappingErrors, 0);
}

TEST_F(DreamBaseTest, DataReceiveBad) {
  Settings.DetectorPort = 9001;
  Settings.UpdateIntervalSec = 0;
  DreamBaseStandIn Readout(Settings);
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43127, Settings.DetectorPort,
                       (unsigned char *)&TestPacket3[0], TestPacket3.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.Counters.RxPackets, 1);
  EXPECT_EQ(Readout.Counters.RxBytes, TestPacket3.size());
  EXPECT_EQ(Readout.Counters.Readouts, 0);
  EXPECT_EQ(Readout.Counters.DataHeaders, 0);
  EXPECT_EQ(Readout.Counters.GeometryErrors, 0);
  EXPECT_EQ(Readout.Counters.MappingErrors, 0);
  EXPECT_EQ(Readout.Counters.ErrorESSHeaders, 1);
}

int main(int argc, char **argv) {
  std::string filename{"deleteme_dream.json"};
  saveBuffer(filename, (void *)dreamjson.c_str(), dreamjson.size());

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
