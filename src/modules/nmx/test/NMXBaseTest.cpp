// Copyright (C) 2021 - 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for NMXBase
///
//===----------------------------------------------------------------------===//

#include <cinttypes>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <nmx/NMXBase.h>
#include <string>
#include <vector>

// clang-format off

std::vector<uint8_t> TestPacket {
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

class NMXBaseTest : public ::testing::Test {
public:
  BaseSettings Settings;
  std::chrono::duration<std::int64_t, std::milli> SleepTime{750};

  /// \brief utility function to emulate reception of an UDP packet into
  /// the ringbuffer (to avoid using socket calls)
  void writePacketToRxFIFO(Nmx::NmxBase & Base, std::vector<uint8_t> Packet) {
    Base.startThreads();

    unsigned int rxBufferIndex = Base.RxRingbuffer.getDataIndex();
    ASSERT_EQ(rxBufferIndex, 0);
    auto PacketSize = Packet.size();

    Base.RxRingbuffer.setDataLength(rxBufferIndex, PacketSize);
    auto DataPtr = Base.RxRingbuffer.getDataBuffer(rxBufferIndex);
    memcpy(DataPtr, (unsigned char *)&Packet[0], PacketSize);

    ASSERT_TRUE(Base.InputFifo.push(rxBufferIndex));
    Base.RxRingbuffer.getNextBuffer();

    std::this_thread::sleep_for(SleepTime);
    Base.stopThreads();
  }

  void SetUp() override {
    Settings.ConfigFile = "nmx.json";
    Settings.NoHwCheck = true;
  }

  void TearDown() override {}
};

TEST_F(NMXBaseTest, Constructor) {
  Nmx::NmxBase Readout(Settings);
  EXPECT_EQ(Readout.Counters.VMMStats.Readouts, 0);
}

TEST_F(NMXBaseTest, DataReceive) {
  Nmx::NmxBase Readout(Settings);

  writePacketToRxFIFO(Readout, TestPacket);

  EXPECT_EQ(Readout.Counters.VMMStats.Readouts, 2); // # readouts in TestPacket
  EXPECT_EQ(Readout.Counters.VMMStats.DataReadouts, 2);
  // this instance of NMX has 4 event builders, each attempts
  // matching once per packet received, so expecting counter to be 4
  EXPECT_EQ(Readout.Counters.MatcherStats.MatchAttemptCount, 4);
}

TEST_F(NMXBaseTest, DataReceiveBadHeader) {
  Nmx::NmxBase Readout(Settings);

  TestPacket[0] = 0xff; // pad should be 0
  writePacketToRxFIFO(Readout, TestPacket);

  EXPECT_EQ(Readout.Counters.ErrorESSHeaders, 1);
  EXPECT_EQ(Readout.Counters.VMMStats.Readouts, 0); // no readouts: bad header
  EXPECT_EQ(Readout.Counters.VMMStats.DataReadouts, 0);
}

int main(int argc, char **argv) {
  std::string filename{"nmx.json"};
  saveBuffer(filename, (void *)nmxjson.c_str(), nmxjson.size());

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
