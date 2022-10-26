// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief test for class MiraclesBase
///
//===----------------------------------------------------------------------===//

#include <string>

#include <miracles/MiraclesBase.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <common/testutils/TestUDPServer.h>

std::string miraclesjson = R"(
  {
    "Detector" : "Miracles",

    "MaxPulseTimeNS" : 357142855
  }
)";

class MiraclesBaseStandIn : public Miracles::MiraclesBase {
public:
  MiraclesBaseStandIn(BaseSettings Settings)
      : Miracles::MiraclesBase(Settings){};
  ~MiraclesBaseStandIn() = default;
  using Miracles::MiraclesBase::Counters;
  using Detector::Threads;
};

class MiraclesBaseTest : public ::testing::Test {
public:
  void SetUp() override {
    Settings.RxSocketBufferSize = 100000;
    Settings.NoHwCheck = true;
    Settings.ConfigFile = "deleteme_miracles.json";
  }
  void TearDown() override {}

  std::chrono::duration<std::int64_t, std::milli> SleepTime{400};
  BaseSettings Settings;
};

TEST_F(MiraclesBaseTest, Constructor) {
  MiraclesBaseStandIn Readout(Settings);
  EXPECT_EQ(Readout.Counters.RxPackets, 0);
  EXPECT_EQ(Readout.Counters.RxBytes, 0);

  EXPECT_EQ(Readout.Counters.ErrorESSHeaders, 0);
  EXPECT_EQ(Readout.Counters.ReadoutStats.HeartBeats, 0);

  EXPECT_EQ(Readout.Counters.DataHeaders, 0);
  EXPECT_EQ(Readout.Counters.ErrorDataHeaders, 0);
  EXPECT_EQ(Readout.Counters.Readouts, 0);
  EXPECT_EQ(Readout.Counters.RingErrors, 0);
}

std::vector<uint8_t> TestPacket2{
    // ESS header
    0x00, 0x00,             // pad, v0
    0x45, 0x53, 0x53, 0x38, // 'E' 'S' 'S' 0x60
    0x36, 0x00, 0x00, 0x00, // 0x002e - 54 bytes
    0x11, 0x00, 0x00, 0x00, // Pulse time High (17s)
    0x00, 0x01, 0x00, 0x00, // Pulse time Low (256 clocks)
    0x11, 0x00, 0x00, 0x00, //
    0x00, 0x01, 0x00, 0x00, //
    0x01, 0x00, 0x00, 0x00, // Seq number 1

    // Data Header 1
    0x00, 0x00, 0x18, 0x00, // ring 0, fen 0, data size 24 bytes

    // Readout 1
    0x11, 0x00, 0x00, 0x00, // time high 17 (0x11)
    0x00, 0x02, 0x00, 0x00, // time low 512 (0x00 00 02 00)
    0xf4, 0x00, 0x00, 0x00, // flags set, miracles (0xf4)
    0xaa, 0xaa, 0xbb, 0xbb, // AmpA: 0xaaaa, AmpB: 0xbbbb
    0x00, 0x00, 0x00, 0x00  // Unused
};

TEST_F(MiraclesBaseTest, DataReceiveGood) {
  Settings.DetectorPort = 9001;
  Settings.UpdateIntervalSec = 0;
  Settings.DumpFilePrefix = "deleteme_miraclesbasetest_";
  MiraclesBaseStandIn Readout(Settings);
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
}

TEST_F(MiraclesBaseTest, HeaderError) {
  Settings.DetectorPort = 9001;
  Settings.UpdateIntervalSec = 0;
  Settings.DumpFilePrefix = "deleteme_miraclesbasetest_";
  MiraclesBaseStandIn Readout(Settings);
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestPacket2[5] = 0x00; // wrong type == header error
  TestUDPServer Server(43127, Settings.DetectorPort,
                       (unsigned char *)&TestPacket2[0], TestPacket2.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.Counters.RxPackets, 1);
  EXPECT_EQ(Readout.Counters.ErrorESSHeaders, 1);
  EXPECT_EQ(Readout.Counters.RxBytes, TestPacket2.size());
  EXPECT_EQ(Readout.Counters.Readouts, 0);
}

int main(int argc, char **argv) {
  std::string filename{"deleteme_miracles.json"};
  saveBuffer(filename, (void *)miraclesjson.c_str(), miraclesjson.size());

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
