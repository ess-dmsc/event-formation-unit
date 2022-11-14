// Copyright (C) 2021 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for NMXBase
///
//===----------------------------------------------------------------------===//

#include <cinttypes>
#include <string>
#include <vector>

// clang-format off

std::vector<uint8_t> dummyreadout {
              0x00, 0x00, // pad, v0
  0x45, 0x53, 0x53, 0x44, // 'E', 'S', 'S', type 0x44
  0x46, 0x00, 0x17, 0x00, // len(0x005e), OQ23, TSrc0
  0x00, 0x00, 0x00, 0x00, // PT HI
  0x00, 0x00, 0x00, 0x00, // PT LO
  0x00, 0x00, 0x00, 0x00, // PPT HI
  0x00, 0x00, 0x00, 0x00, // PPT Lo
  0x08, 0x00, 0x00, 0x00, // Seq number 8

   // First readout - plane Y
  0x00, 0x01, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3C,  // GEO 0, TDC 0, VMM 1, CH 60

  // Second readout - plane X
  0x00, 0x01, 0x14, 0x00,  // Data Header, Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60
};

 std::string nmxjson = R"(
  {
  "Detector" : "NMX",
  "InstrumentGeometry" : "NMX",
  "MaxSpanX" : 10,
  "MaxSpanY" : 10,
  "MaxGapX" : 2,
  "MaxGapY" : 2,
  "DefaultMinADC":50,
  "Config" : [
        {
          "Ring" :  0,
          "FEN": 0,
          "Hybrid" :  0,
          "Plane" : 0,
          "Offset" : 0,
          "ReversedChannels" : true,
          "Panel" : 0,
          "HybridId" : "E5533333222222221111111100000000"
        },
        {
          "Ring" :  0,
          "FEN": 0,
          "Hybrid" :  1,
          "Plane" : 0,
          "Offset" : 128,
          "ReversedChannels" : true,
          "Panel" : 0,
          "HybridId" : "E5533333222222221111111100000001"
        },
        {
          "Ring" :  0,
          "FEN": 0,
          "Hybrid" :  2,
          "Plane" : 0,
          "Offset" : 256,
          "ReversedChannels" : true,
          "Panel" : 0,
          "HybridId" : "E5533333222222221111111100000002"
        }
      ]
  }
)";

// clang-format on

#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <common/testutils/TestUDPServer.h>
#include <nmx/NMXBase.h>

class NMXBaseStandIn : public Nmx::NmxBase {
public:
  NMXBaseStandIn(BaseSettings Settings) : Nmx::NmxBase(Settings){};
  ~NMXBaseStandIn() = default;
  using Detector::Threads;
  using Nmx::NmxBase::Counters;
};

class NMXBaseTest : public ::testing::Test {
public:
  void SetUp() override {
    Settings.ConfigFile = "nmx.json";
    Settings.RxSocketBufferSize = 100000;
    Settings.NoHwCheck = true;
  }
  void TearDown() override {}

  BaseSettings Settings;
};

TEST_F(NMXBaseTest, Constructor) {
  NMXBaseStandIn Readout(Settings);
  EXPECT_EQ(Readout.Counters.RxPackets, 0);
  EXPECT_EQ(Readout.Counters.VMMStats.Readouts, 0);
}

TEST_F(NMXBaseTest, DataReceive) {
  NMXBaseStandIn Readout(Settings);
  Readout.startThreads();
  std::chrono::duration<std::int64_t, std::milli> SleepTime{400};
  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort,
                       (unsigned char *)&dummyreadout[0], dummyreadout.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.Counters.RxPackets, 1);
  EXPECT_EQ(Readout.Counters.RxBytes, dummyreadout.size());
  EXPECT_EQ(Readout.Counters.VMMStats.Readouts,
            2); // number of readouts dummyreadout
  EXPECT_EQ(Readout.Counters.VMMStats.DataReadouts, 2);
}

TEST_F(NMXBaseTest, DataReceiveBadHeader) {
  NMXBaseStandIn Readout(Settings);
  Readout.startThreads();
  std::chrono::duration<std::int64_t, std::milli> SleepTime{400};
  std::this_thread::sleep_for(SleepTime);
  dummyreadout[0] = 0xff; // pad should be 0
  TestUDPServer Server(43126, Settings.DetectorPort,
                       (unsigned char *)&dummyreadout[0], dummyreadout.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.Counters.RxPackets, 1);
  EXPECT_EQ(Readout.Counters.RxBytes, dummyreadout.size());
  EXPECT_EQ(Readout.Counters.ErrorESSHeaders, 1);
  EXPECT_EQ(Readout.Counters.VMMStats.Readouts,
            0); // no readouts as header is bad
  EXPECT_EQ(Readout.Counters.VMMStats.DataReadouts, 0);
}

int main(int argc, char **argv) {
  std::string filename{"nmx.json"};
  saveBuffer(filename, (void *)nmxjson.c_str(), nmxjson.size());

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
