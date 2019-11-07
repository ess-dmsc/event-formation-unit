/** Copyright (C) 2019 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include <string>

#include <loki/LokiBase.h>
#include <../src/adc_readout/test/TestUDPServer.h>
#include <test/TestBase.h>

class LokiBaseStandIn : public Loki::LokiBase {
public:
  LokiBaseStandIn(BaseSettings Settings, struct Loki::LokiSettings ReadoutSettings)
      : Loki::LokiBase(Settings, ReadoutSettings){};
  ~LokiBaseStandIn() = default;
  using Detector::Threads;
  using Loki::LokiBase::Counters;
};

class LokiBaseTest : public ::testing::Test {
public:
  void SetUp() override {
    Settings.DetectorRxBufferSize = 100000;
    Settings.NoHwCheck = true;
  }
  void TearDown() override {}

  std::chrono::duration<std::int64_t, std::milli> SleepTime{400};
  BaseSettings Settings;
  Loki::LokiSettings LocalSettings;
};

TEST_F(LokiBaseTest, Constructor) {
  LokiBaseStandIn Readout(Settings, LocalSettings);
  EXPECT_EQ(Readout.Counters.RxPackets, 0);
}


std::vector<uint8_t> TestPacket{0x00, 0x01, 0x02};

std::vector<uint8_t> TestPacket2{
    0x45, 0x53, 0x53, 0x00, //  'E' 'S' 'S' 0x00
    0x30, 0x00, 0x9c, 0x00, // 0x009c = 156
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, // Seq number 1

    0x00, 0x00, 0x40, 0x00, // Data Header, ring 0, fen 0

    0x00, 0x00, 0x00, 0x00, // Readout 1, time 0
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x02, 0x01,
    0x03, 0x01, 0x04, 0x01,

    0x00, 0x00, 0x00, 0x00, // Readout 2
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x02, 0x02,
    0x03, 0x02, 0x04, 0x02,

    0x00, 0x00, 0x00, 0x00, // Readout 3
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x03, 0x02, 0x03,
    0x03, 0x03, 0x04, 0x03,

    0x01, 0x01, 0x40, 0x00, // Data Header 2, ring 1, fen 1

    0x01, 0x00, 0x00, 0x00, // Readout 1, time 1
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x02, 0x01,
    0x03, 0x01, 0x04, 0x01,

    0x01, 0x00, 0x00, 0x00, // Readout 2
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x02, 0x02,
    0x03, 0x02, 0x04, 0x02,

    0x01, 0x00, 0x00, 0x00, // Readout 3
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x03, 0x02, 0x03,
    0x03, 0x03, 0x04, 0x03,
};

TEST_F(LokiBaseTest, DataReceive) {
  Settings.DetectorPort = 9000;
  LokiBaseStandIn Readout(Settings, LocalSettings);
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort, (unsigned char *)&TestPacket[0], TestPacket.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.Counters.RxPackets, 1);
  EXPECT_EQ(Readout.Counters.RxBytes, TestPacket.size());
  EXPECT_EQ(Readout.Counters.Readouts, 0);
}

TEST_F(LokiBaseTest, DataReceiveGood) {
  Settings.DetectorPort = 9001;
  LokiBaseStandIn Readout(Settings, LocalSettings);
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43127, Settings.DetectorPort, (unsigned char *)&TestPacket2[0], TestPacket2.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.Counters.RxPackets, 1);
  EXPECT_EQ(Readout.Counters.RxBytes, TestPacket2.size());
  EXPECT_EQ(Readout.Counters.Readouts, 6);
  EXPECT_EQ(Readout.Counters.Headers, 2);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
