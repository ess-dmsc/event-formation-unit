// Copyright (C) 2019 - 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for CaenBase
///
//===----------------------------------------------------------------------===//

#include <string>

#include <caen/CaenBase.h>
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
std::string lokijson = R"(
{
  "Detector" : "LoKI",

  "StrawResolution" : 512,

  "PanelConfig" : [
    { "Bank" : 0, "Vertical" :  true,  "TubesZ" : 4, "TubesN" : 8, "StrawOffset" :   0 },
    { "Bank" : 1, "Vertical" :  false, "TubesZ" : 4, "TubesN" : 8, "StrawOffset" : 224 }
  ],
  "MaxTOFNS" : 800000000,
  "MaxRing" : 2
}
)";

std::string bifrostjson = R"(
  {
    "Detector": "BIFROST",
    "MaxRing": 2,
    "StrawResolution": 300
  }
)";

std::string miraclesjson = R"(
  {
    "Detector": "Miracles",
    "MaxRing": 2,
    "StrawResolution": 128
  }
)";

class CaenBaseStandIn : public Caen::CaenBase {
public:
  CaenBaseStandIn(BaseSettings Settings, struct Caen::CaenSettings ReadoutSettings)
      : Caen::CaenBase(Settings, ReadoutSettings){};
  ~CaenBaseStandIn() = default;
  using Detector::Threads;
  using Caen::CaenBase::Counters;
};

class CaenBaseTest : public ::testing::Test {
public:
  void SetUp() override {
    Settings.RxSocketBufferSize = 100000;
    Settings.NoHwCheck = true;
    LocalSettings.ConfigFile = "deleteme_loki.json";
  }
  void TearDown() override {}

  std::chrono::duration<std::int64_t, std::milli> SleepTime{400};
  BaseSettings Settings;
  Caen::CaenSettings LocalSettings;
};

TEST_F(CaenBaseTest, LokiConstructor) {
  CaenBaseStandIn Readout(Settings, LocalSettings);
  EXPECT_EQ(Readout.Counters.RxPackets, 0);
}

TEST_F(CaenBaseTest, BifrostConstructor) {
  LocalSettings.ConfigFile = "deleteme_bifrost.json";
  CaenBaseStandIn Readout(Settings, LocalSettings);
  EXPECT_EQ(Readout.Counters.RxPackets, 0);
}

TEST_F(CaenBaseTest, MiraclesConstructor) {
  LocalSettings.ConfigFile = "deleteme_miracles.json";
  CaenBaseStandIn Readout(Settings, LocalSettings);
  EXPECT_EQ(Readout.Counters.RxPackets, 0);
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

TEST_F(CaenBaseTest, DataReceiveLoki) {
  Settings.DetectorPort = 9000;
  CaenBaseStandIn Readout(Settings, LocalSettings);
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort,
                       (unsigned char *)&TestPacket[0], TestPacket.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.Counters.RxPackets, 1);
  EXPECT_EQ(Readout.Counters.RxBytes, TestPacket.size());
  EXPECT_EQ(Readout.Counters.Readouts, 0);
}

TEST_F(CaenBaseTest, DataReceiveBifrost) {
  Settings.DetectorPort = 9000;
  LocalSettings.ConfigFile = "deleteme_bifrost.json";
  CaenBaseStandIn Readout(Settings, LocalSettings);
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort,
                       (unsigned char *)&TestPacket[0], TestPacket.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.Counters.RxPackets, 1);
  EXPECT_EQ(Readout.Counters.RxBytes, TestPacket.size());
  EXPECT_EQ(Readout.Counters.Readouts, 0);
}

TEST_F(CaenBaseTest, DataReceiveMiracles) {
  Settings.DetectorPort = 9000;
  LocalSettings.ConfigFile = "deleteme_miracles.json";
  CaenBaseStandIn Readout(Settings, LocalSettings);
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort,
                       (unsigned char *)&TestPacket[0], TestPacket.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.Counters.RxPackets, 1);
  EXPECT_EQ(Readout.Counters.RxBytes, TestPacket.size());
  EXPECT_EQ(Readout.Counters.Readouts, 0);
}

TEST_F(CaenBaseTest, DataReceiveGoodLoki) {
  XTRACE(DATA, DEB, "Running DataReceiveGood test");
  Settings.DetectorPort = 9001;
  Settings.UpdateIntervalSec = 0;
  LocalSettings.FilePrefix = "deleteme_";
  CaenBaseStandIn Readout(Settings, LocalSettings);
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43127, Settings.DetectorPort,
                       (unsigned char *)&TestPacket2[0], TestPacket2.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.Counters.RxPackets, 1);
  EXPECT_EQ(Readout.Counters.RxBytes, TestPacket2.size());
  EXPECT_EQ(Readout.Counters.Readouts, 6);
  EXPECT_EQ(Readout.Counters.DataHeaders, 6);
  EXPECT_EQ(Readout.Counters.PixelErrors, 1);
  EXPECT_EQ(Readout.Counters.RingErrors, 1);
  EXPECT_EQ(Readout.Counters.FENErrors, 1);
  EXPECT_EQ(Readout.Counters.TofHigh, 1);
  EXPECT_EQ(Readout.Counters.PrevTofNegative, 1);
}

TEST_F(CaenBaseTest, DataReceiveGoodBifrost) {
  XTRACE(DATA, DEB, "Running DataReceiveGood test");
  LocalSettings.ConfigFile = "deleteme_bifrost.json";
  Settings.DetectorPort = 9001;
  Settings.UpdateIntervalSec = 0;
  LocalSettings.FilePrefix = "deleteme_";
  CaenBaseStandIn Readout(Settings, LocalSettings);
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43127, Settings.DetectorPort,
                       (unsigned char *)&TestPacket2[0], TestPacket2.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.Counters.RxPackets, 1);
  EXPECT_EQ(Readout.Counters.RxBytes, TestPacket2.size());
  EXPECT_EQ(Readout.Counters.Readouts, 6);
  EXPECT_EQ(Readout.Counters.DataHeaders, 6);
  EXPECT_EQ(Readout.Counters.PixelErrors, 1);
  EXPECT_EQ(Readout.Counters.RingErrors, 1);
  EXPECT_EQ(Readout.Counters.TofHigh, 1);
  EXPECT_EQ(Readout.Counters.PrevTofNegative, 1);
}

int main(int argc, char **argv) {
  std::string lokifilename{"deleteme_loki.json"};
  saveBuffer(lokifilename, (void *)lokijson.c_str(), lokijson.size());
  std::string bifrostfilename{"deleteme_bifrost.json"};
  saveBuffer(bifrostfilename, (void *)bifrostjson.c_str(), bifrostjson.size());
  std::string miraclesfilename{"deleteme_miracles.json"};
  saveBuffer(miraclesfilename, (void *)miraclesjson.c_str(), miraclesjson.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(lokifilename);
  deleteFile(bifrostfilename);
  deleteFile(miraclesfilename);
  return RetVal;
}
