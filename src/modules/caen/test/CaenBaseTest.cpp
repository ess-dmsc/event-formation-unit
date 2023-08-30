// Copyright (C) 2019 - 2023 European Spallation Source, see LICENSE file
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
#include <common/testutils/TestBase.h>
#include <common/testutils/TestUDPServer.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

class CaenBaseTest : public ::testing::Test {
public:
  void SetUp() override {
    Settings.RxSocketBufferSize = 100000;
    Settings.NoHwCheck = true;
    Settings.ConfigFile =  LOKI_CONFIG;
    Settings.CalibFile = LOKI_CALIB;
  }
  void TearDown() override {}
  std::chrono::duration<std::int64_t, std::milli> SleepTime{400};
  BaseSettings Settings;
};

TEST_F(CaenBaseTest, LokiConstructor) {
  Settings.DetectorName = "loki";
  Caen::CaenBase Readout(Settings, ESSReadout::Parser::LOKI);
  EXPECT_EQ(Readout.ITCounters.RxPackets, 0);
}

TEST_F(CaenBaseTest, BifrostConstructor) {
  Settings.ConfigFile = BIFROST_CONFIG;
  Settings.CalibFile = BIFROST_CALIB;
  Settings.DetectorName = "bifrost";
  Caen::CaenBase Readout(Settings, ESSReadout::Parser::BIFROST);
  Readout.Counters = {};
  EXPECT_EQ(Readout.ITCounters.RxPackets, 0);
}

TEST_F(CaenBaseTest, MiraclesConstructor) {
  Settings.ConfigFile = MIRACLES_CONFIG;
  Settings.CalibFile = MIRACLES_CALIB;
  Settings.DetectorName = "miracles";
  Caen::CaenBase Readout(Settings, ESSReadout::Parser::MIRACLES);
  Readout.Counters = {};
  EXPECT_EQ(Readout.ITCounters.RxPackets, 0);
}

std::vector<uint8_t> TestPacket{0x00, 0x01, 0x02};

///
std::vector<uint8_t> TestPacket2{
    // ESS header
                0x00, 0x00, // pad, v0
    0x45, 0x53, 0x53, 0x30, //  'E' 'S' 'S' 0x30
    0xae, 0x00, 0x00, 0x00, // 0x96 = 150 bytes
    0x11, 0x00, 0x00, 0x00, // Pulse time High (17s)
    0x00, 0x01, 0x00, 0x00, // Pulse time Low (256 clocks)
    0x11, 0x00, 0x00, 0x00, // Prev PT
    0x00, 0x00, 0x00, 0x00, //
    0x01, 0x00, 0x00, 0x00, // Seq number 1


    // Data Header 1
    0x00, 0x00, 0x18, 0x00, // fiber 0, fen 0, data size 64 bytes
    // Readout
    0x11, 0x00, 0x00, 0x00, // time high (17s)
    0x01, 0x01, 0x00, 0x00, // time low (257 clocks)
    0x00, 0x00, 0x00, 0x00, // fpga 0, tube 0
    0x01, 0x01, 0x02, 0x01, // amp a, amp b
    0x03, 0x01, 0x04, 0x01, // amp c, amp d


    // Data Header 2
    // Fiber 20 (Ring 10) is invalid -> RingErrors++
    0x14, 0x00, 0x18, 0x00, // fiber 20, fen 0, data size 64 bytes
    // Readout
    0x11, 0x00, 0x00, 0x00, //time high 17s
    0x01, 0x02, 0x00, 0x00, // time low (257 clocks)
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x02, 0x01,
    0x03, 0x01, 0x04, 0x01,

    // Data Header 3
    // FEN 19 is invalid -> FENErrors++ (for loki only so far)
    0x01, 0x13, 0x18, 0x00, // fiber 1, fen 19, size 24 bytes
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
  Settings.DetectorName = "loki";

  Settings.DetectorPort = 9210;
  Caen::CaenBase Readout(Settings, ESSReadout::Parser::LOKI);
  Readout.Counters = {};
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort,
                       (unsigned char *)&TestPacket[0], TestPacket.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.ITCounters.RxPackets, 1);
  EXPECT_EQ(Readout.ITCounters.RxBytes, TestPacket.size());
  EXPECT_EQ(Readout.Counters.Parser.Readouts, 0);
}

TEST_F(CaenBaseTest, DataReceiveBifrost) {
  Settings.DetectorName = "bifrost";
  Settings.ConfigFile = BIFROST_CONFIG;
  Settings.CalibFile = BIFROST_CALIB;

  Settings.DetectorPort = 9211;
  Caen::CaenBase Readout(Settings, ESSReadout::Parser::BIFROST);
  Readout.Counters = {};
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort,
                       (unsigned char *)&TestPacket[0], TestPacket.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.ITCounters.RxPackets, 1);
  EXPECT_EQ(Readout.ITCounters.RxBytes, TestPacket.size());
  EXPECT_EQ(Readout.Counters.Parser.Readouts, 0);
}

TEST_F(CaenBaseTest, DataReceiveMiracles) {
  Settings.DetectorName = "miracles";
  Settings.ConfigFile = MIRACLES_CONFIG;
  Settings.CalibFile = MIRACLES_CALIB;

  Settings.DetectorPort = 9213;
  Caen::CaenBase Readout(Settings, ESSReadout::Parser::MIRACLES);
  Readout.Counters = {};
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort,
                       (unsigned char *)&TestPacket[0], TestPacket.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.ITCounters.RxPackets, 1);
  EXPECT_EQ(Readout.ITCounters.RxBytes, TestPacket.size());
  EXPECT_EQ(Readout.Counters.Parser.Readouts, 0);
}

TEST_F(CaenBaseTest, DataReceiveGoodLoki) {
  XTRACE(DATA, DEB, "Running DataReceiveGood test");
  Settings.DetectorName = "loki";

  Settings.DetectorPort = 9214;
  Settings.UpdateIntervalSec = 0;
  Settings.DumpFilePrefix = "deleteme_";
  Caen::CaenBase Readout(Settings, ESSReadout::Parser::LOKI);
  Readout.Counters = {};
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43127, Settings.DetectorPort,
                       (unsigned char *)&TestPacket2[0], TestPacket2.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.ITCounters.RxPackets, 1);
  EXPECT_EQ(Readout.ITCounters.RxBytes, TestPacket2.size());
  EXPECT_EQ(Readout.Counters.Parser.Readouts, 6);
  EXPECT_EQ(Readout.Counters.Parser.DataHeaders, 6);
  EXPECT_EQ(Readout.Counters.PixelErrors, 1);
  EXPECT_EQ(Readout.Counters.Geom.RingMappingErrors, 1);
  EXPECT_EQ(Readout.Counters.TimeStats.TofHigh, 1);
  EXPECT_EQ(Readout.Counters.TimeStats.PrevTofNegative, 1);
}

TEST_F(CaenBaseTest, DataReceiveGoodBifrost) {
  XTRACE(DATA, DEB, "Running DataReceiveGood test");
  Settings.DetectorName = "bifrost";
  Settings.ConfigFile = BIFROST_CONFIG;
  Settings.CalibFile = BIFROST_CALIB;

  Settings.DetectorPort = 9215;
  Settings.UpdateIntervalSec = 0;
  Settings.DumpFilePrefix = "deleteme_";
  Caen::CaenBase Readout(Settings, ESSReadout::Parser::BIFROST);
  Readout.Counters = {};
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43127, Settings.DetectorPort,
                       (unsigned char *)&TestPacket2[0], TestPacket2.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.ITCounters.RxPackets, 1);
  EXPECT_EQ(Readout.ITCounters.RxBytes, TestPacket2.size());
}

TEST_F(CaenBaseTest, DataReceiveGoodMiracles) {
  XTRACE(DATA, DEB, "Running DataReceiveGood test");
  Settings.DetectorName = "miracles";
  Settings.ConfigFile = MIRACLES_CONFIG;
  Settings.CalibFile = MIRACLES_CALIB;

  Settings.DetectorPort = 9216;
  Settings.UpdateIntervalSec = 0;
  Settings.DumpFilePrefix = "deleteme_";
  Caen::CaenBase Readout(Settings, ESSReadout::Parser::MIRACLES);
  Readout.Counters = {};
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43127, Settings.DetectorPort,
                       (unsigned char *)&TestPacket2[0], TestPacket2.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.ITCounters.RxPackets, 1);
  EXPECT_EQ(Readout.ITCounters.RxBytes, TestPacket2.size());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
