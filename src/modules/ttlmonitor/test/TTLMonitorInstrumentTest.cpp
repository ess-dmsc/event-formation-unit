// Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/kafka/EV44Serializer.h>
#include <common/readout/ess/Parser.h>
#include <common/reduction/Event.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <stdio.h>
#include <string.h>
#include <ttlmonitor/TTLMonitorInstrument.h>

using namespace TTLMonitor;

// clang-format off
std::string ConfigFile{"deleteme_ttlmonitor_instr_config.json"};
std::string ConfigStr = R"(
  {
    "Detector" : "TTLMonitor",

    "TypeSubType" : 72,

    "MaxPulseTimeDiffNS" : 1000000000,

    "MaxTOFNS" : 1000000000,

    "NumberOfMonitors" : 3
  }
)";

std::vector<uint8_t> MonitorReadout {
  // Errors caught when parsing readouts

  // First monitor readout - Valid
  0x16, 0x00, 0x10, 0x00,  // Data Header - PRing 22, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x01, 0x00,  // Pos 0, Ch 0, ADC 1

  // Second monitor readout - invalid Ring
  0x18, 0x00, 0x10, 0x00,  // Data Header, PRing 24, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x00, 0x00, 0x01, 0x00,  // Pos 0, Ch 0, ADC 1

  // Third monitor readout - invalid FEN
  0x17, 0x22, 0x10, 0x00,  // Data Header, PRing 23, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x00, 0x00, 0x01, 0x00,  // Pos 0, Ch 0, ADC 1

  // Fourth monitor readout - invalid Channel
  0x17, 0x00, 0x10, 0x00,  // Data Header, PRing 23, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x00, 0x07, 0x01, 0x00,  // Pos 0, Ch 7, ADC 1

  // Fifth monitor readout - invalid ADC
  0x17, 0x00, 0x10, 0x00,  // Data Header, PRing 23, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x00, 0x00, 0x00, 0x00,  // Pos 0, Ch 0, ADC 0

  // Errors caught when processing readouts

  // Sixth monitor readout - invalid RingCfg
  0x12, 0x00, 0x10, 0x00,  // Data Header, PRing 18, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x00, 0x00, 0x01, 0x00,  // Pos 0, Ch 0, ADC 1

  // Seventh monitor readout - invalid FENCfg
  0x17, 0x01, 0x10, 0x00,  // Data Header, PRing 18, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x00, 0x00, 0x01, 0x00,  // Pos 0, Ch 0, ADC 1
};


std::vector<uint8_t> MonitorReadoutTOF {
  // First monitor readout - Negative PrevTOF - possibly unreachable!
  0x16, 0x00, 0x10, 0x00,  // Data Header - Ring 22, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x00,  // 0x00000000

  // Second monitor readout - Negative TOF, positive PrevTOF
  0x16, 0x00, 0x10, 0x00,  // Data Header - Ring 22, FEN 0
  0x01, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x00,  // 0x00000000
};
// clang-format on

class TTLMonitorInstrumentTest : public TestBase {
public:
protected:
  struct Counters counters;
  BaseSettings Settings;
  std::vector<EV44Serializer> serializers;
  TTLMonitorInstrument *ttlmonitor;
  ESSReadout::Parser::PacketHeaderV0 PacketHeader;
  Event TestEvent;           // used for testing generateEvents()
  std::vector<Event> Events; // used for testing generateEvents()

  void SetUp() override {
    Settings.ConfigFile = ConfigFile;
    serializers.push_back(EV44Serializer(115000, "ttlmonitor"));
    counters = {};

    memset(&PacketHeader, 0, sizeof(PacketHeader));

    ttlmonitor = new TTLMonitorInstrument(counters, Settings);
    ttlmonitor->Serializers.push_back(&serializers[0]);
    ttlmonitor->ESSReadoutParser.Packet.HeaderPtr = &PacketHeader;
  }
  void TearDown() override {}

  void makeHeader(ESSReadout::Parser::PacketDataV0 &Packet,
                  std::vector<uint8_t> &testdata) {
    Packet.HeaderPtr = &PacketHeader;
    Packet.DataPtr = (char *)&testdata[0];
    Packet.DataLength = testdata.size();
    Packet.Time.setReference(0, 0);
    Packet.Time.setPrevReference(0, 0);
  }
};

// Test cases below
TEST_F(TTLMonitorInstrumentTest, Constructor) {
  ASSERT_EQ(counters.RingCfgErrors, 0);
  ASSERT_EQ(counters.FENCfgErrors, 0);
}

TEST_F(TTLMonitorInstrumentTest, BeamMonitor) {
  makeHeader(ttlmonitor->ESSReadoutParser.Packet, MonitorReadout);

  ttlmonitor->TTLMonParser.parse(ttlmonitor->ESSReadoutParser.Packet);
  counters.TTLMonStats = ttlmonitor->TTLMonParser.Stats;

  ASSERT_EQ(counters.TTLMonStats.Readouts, 7);
  ASSERT_EQ(counters.TTLMonStats.ErrorRing, 1);
  ASSERT_EQ(counters.TTLMonStats.ErrorFEN, 1);
  ASSERT_EQ(counters.TTLMonStats.ErrorADC, 1);

  ttlmonitor->processMonitorReadouts();
  ASSERT_EQ(counters.RingCfgErrors, 1);
  ASSERT_EQ(counters.FENCfgErrors, 1);
  ASSERT_EQ(counters.MonitorCounts, 2);

  ASSERT_EQ(counters.FENCfgErrors, 1);
}

TEST_F(TTLMonitorInstrumentTest, BeamMonitorTOF) {
  makeHeader(ttlmonitor->ESSReadoutParser.Packet, MonitorReadoutTOF);
  ttlmonitor->ESSReadoutParser.Packet.Time.setReference(1, 100000);
  ttlmonitor->ESSReadoutParser.Packet.Time.setPrevReference(1, 0);

  ttlmonitor->TTLMonParser.parse(ttlmonitor->ESSReadoutParser.Packet);
  counters.TTLMonStats = ttlmonitor->TTLMonParser.Stats;

  ttlmonitor->processMonitorReadouts();
}

int main(int argc, char **argv) {
  saveBuffer(ConfigFile, (void *)ConfigStr.c_str(), ConfigStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(ConfigFile);
  return RetVal;
}
