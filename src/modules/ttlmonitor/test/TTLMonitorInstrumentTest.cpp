// Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/kafka/EV42Serializer.h>
#include <common/reduction/Event.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <ttlmonitor/TTLMonitorInstrument.h>
#include <stdio.h>
#include <string.h>

using namespace TTLMonitor;

std::string ConfigFile{"deleteme_ttlmonitor_instr_config.json"};
std::string ConfigStr = R"(
  {
    "Detector" : "TTLMonitor",

    "TypeSubType" : 72,

    "MaxPulseTimeDiffNS" : 1000000000,

    "MaxTOFNS" : 1000000000
  }
)";

std::vector<uint8_t> MonitorReadout {
  // First monitor readout - Valid
  0x16, 0x00, 0x14, 0x00,  // Data Header - Ring 22, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x00,  // 0x00000000
  0x00, 0x00, 0x00, 0x00,  // 0x00000000

  // Second monitor readout - invalid VMM
  0x17, 0x00, 0x14, 0x00,  // Data Header, Ring 23, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x00, 0x00, 0x00, 0x00,  // 0x00000000
  0x00, 0x00, 0x01, 0x00,  // VMM 1 invalid

  // Third monitor readout - invalid TDC
  0x17, 0x00, 0x14, 0x00,  // Data Header, Ring 23, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x00, 0x00, 0x00, 0x00,  // 0x00000000
  0x00, 0x01, 0x00, 0x00,  // TDC 1 invalid

  // Fourth monitor readout - invalid GEO
  0x17, 0x00, 0x14, 0x00,  // Data Header, Ring 23, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x00, 0x00, 0x00, 0x00,  // 0x00000000
  0x01, 0x00, 0x00, 0x00,  // GEO 1 invalid

  // Fifth monitor readout - invalid BC
  0x17, 0x00, 0x14, 0x00,  // Data Header, Ring 23, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x01, 0x00, 0x00, 0x00,  // BC 1 invalid
  0x00, 0x00, 0x00, 0x00,  // 0x00000000

  // Sixth monitor readout - invalid Ring
  0x15, 0x00, 0x14, 0x00,  // Data Header - Ring 21 invalid, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x00,  // 0x00000000
  0x00, 0x00, 0x00, 0x00,  // 0x00000000

  // Seventh monitor readout - invalid FEN
  0x16, 0x01, 0x14, 0x00,  // Data Header - Ring 22, FEN 1 invalid
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x00,  // 0x00000000
  0x00, 0x00, 0x00, 0x00,  // 0x00000000

  // Eights monitor readout - TOF too large
  0x16, 0x00, 0x14, 0x00,  // Data Header - Ring 22, FEN 0
  0x02, 0x00, 0x00, 0x00,  // Time HI 2 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x00,  // 0x00000000
  0x00, 0x00, 0x00, 0x00,  // 0x00000000
};


std::vector<uint8_t> MonitorReadoutTOF {
  // First monitor readout - Negative PrevTOF - possibly unreachable!
  0x16, 0x00, 0x14, 0x00,  // Data Header - Ring 22, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x00,  // 0x00000000
  0x00, 0x00, 0x00, 0x00,  // 0x00000000

  // Second monitor readout - Negative TOF, positive PrevTOF
  0x16, 0x00, 0x14, 0x00,  // Data Header - Ring 22, FEN 0
  0x01, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x00,  // 0x00000000
  0x00, 0x00, 0x00, 0x00,  // 0x00000000
};


class TTLMonitorInstrumentTest : public TestBase {
public:

protected:
  struct Counters counters;
  TTLMonitorSettings ModuleSettings;
  EV42Serializer * serializer;
  TTLMonitorInstrument * freia;
  ESSReadout::Parser::PacketHeaderV0 PacketHeader;
  Event TestEvent;           // used for testing generateEvents()
  std::vector<Event> Events; // used for testing generateEvents()

  void SetUp() override {
    ModuleSettings.ConfigFile = ConfigFile;
    serializer = new EV42Serializer(115000, "ttlmonitor");
    counters = {};

    memset(&PacketHeader, 0, sizeof(PacketHeader));

    freia = new TTLMonitorInstrument(counters, ModuleSettings, serializer);
    freia->setSerializer(serializer);
    freia->ESSReadoutParser.Packet.HeaderPtr = &PacketHeader;
  }
  void TearDown() override {}

  void makeHeader(ESSReadout::Parser::PacketDataV0 & Packet, std::vector<uint8_t> & testdata) {
    Packet.HeaderPtr = &PacketHeader;
    Packet.DataPtr = (char *)&testdata[0];
    Packet.DataLength = testdata.size();
    Packet.Time.setReference(0,0);
    Packet.Time.setPrevReference(0,0);
  }
};

// Test cases below
TEST_F(TTLMonitorInstrumentTest, Constructor) {
  ASSERT_EQ(counters.RingCfgErrors, 0);
  ASSERT_EQ(counters.FENCfgErrors, 0);
}


TEST_F(TTLMonitorInstrumentTest, BeamMonitor) {
  makeHeader(freia->ESSReadoutParser.Packet, MonitorReadout);

  auto Readouts = freia->VMMParser.parse(freia->ESSReadoutParser.Packet);
  ASSERT_EQ(Readouts, 8);

  freia->processMonitorReadouts();
  ASSERT_EQ(counters.MonitorCounts, 1);
  ASSERT_EQ(counters.MonitorErrors, 4);
  ASSERT_EQ(counters.RingCfgErrors, 1);
  ASSERT_EQ(counters.FENCfgErrors, 1);
  ASSERT_EQ(counters.TOFErrors, 1);
  ASSERT_EQ(counters.VMMStats.ErrorADC, 0);
}

TEST_F(TTLMonitorInstrumentTest, BeamMonitorTOF) {
  makeHeader(freia->ESSReadoutParser.Packet, MonitorReadoutTOF);
  freia->ESSReadoutParser.Packet.Time.setReference(1,100000);
  freia->ESSReadoutParser.Packet.Time.setPrevReference(1,0);

  auto Readouts = freia->VMMParser.parse(freia->ESSReadoutParser.Packet);
  ASSERT_EQ(Readouts, 1);

  freia->processMonitorReadouts();
  ASSERT_EQ(counters.MonitorErrors, 0);
}

int main(int argc, char **argv) {
  saveBuffer(ConfigFile, (void *)ConfigStr.c_str(), ConfigStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(ConfigFile);
  return RetVal;
}
