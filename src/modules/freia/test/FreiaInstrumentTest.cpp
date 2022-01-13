// Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/kafka/EV42Serializer.h>
#include <freia/FreiaInstrument.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <stdio.h>
#include <string.h>

using namespace Freia;

std::string ConfigFile{"deleteme_freia_instr_config.json"};
std::string ConfigStr = R"(
  {
    "Detector": "Freia",

    "WireChOffset" : 16,

    "Config" : [
      { "Ring" :  0, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000000"},
      { "Ring" :  0, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000001"},
      { "Ring" :  0, "FEN": 2, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000002"},
      { "Ring" :  0, "FEN": 2, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000003"},
      { "Ring" :  1, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000004"},
      { "Ring" :  1, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000005"},
      { "Ring" :  1, "FEN": 2, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000006"},
      { "Ring" :  1, "FEN": 2, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000007"},
      { "Ring" :  2, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000008"},
      { "Ring" :  2, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000009"},
      { "Ring" :  2, "FEN": 2, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000010"},
      { "Ring" :  2, "FEN": 2, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000011"},
      { "Ring" :  3, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000012"},
      { "Ring" :  3, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000013"},
      { "Ring" :  4, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000014"},
      { "Ring" :  4, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000015"},
      { "Ring" :  5, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000016"},
      { "Ring" :  5, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000017"},
      { "Ring" :  6, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000018"},
      { "Ring" :  6, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000019"},
      { "Ring" :  7, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000020"},
      { "Ring" :  7, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000021"},
      { "Ring" :  8, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000022"},
      { "Ring" :  8, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000023"},
      { "Ring" :  9, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000024"},
      { "Ring" :  9, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000025"},
      { "Ring" :  9, "FEN": 2, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000026"},
      { "Ring" :  9, "FEN": 2, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000027"},
      { "Ring" : 10, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000028"},
      { "Ring" : 10, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000029"},
      { "Ring" : 10, "FEN": 2, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000030"},
      { "Ring" : 10, "FEN": 2, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000031"}
    ],

    "MaxPulseTimeNS" : 71428570,
    "MaxGapWire"  : 0,
    "MaxGapStrip" : 0,
    "TimeBoxNs" : 2010
  }
)";


//
std::vector<uint8_t> MappingError {
  // First readout
  0x16, 0x00, 0x14, 0x00,  // Data Header - Ring 22!
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x10,  // GEO 0, TDC 0, VMM 0, CH 16

  // Second readout
  0x02, 0x03, 0x14, 0x00,  // Data Header
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticka
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x01, 0x10,  // GEO 0, TDC 0, VMM 1, CH 16
};

std::vector<uint8_t> PixelError {
  // First readout - plane Y - Wires
  0x04, 0x00, 0x14, 0x00,  // Data Header - Ring 4, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x32,  // GEO 0, TDC 0, VMM 0, CH 50

  // Second readout - plane X - Strips
  0x05, 0x00, 0x14, 0x00,  // Data Header, Ring 5, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticka
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x01, 0x10,  // GEO 0, TDC 0, VMM 1, CH 16
};

std::vector<uint8_t> GoodEvent {
  // First readout - plane Y - Wires
  0x04, 0x00, 0x14, 0x00,  // Data Header - Ring 4, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x10,  // GEO 0, TDC 0, VMM 0, CH 16

  // Second readout - plane X - Strips
  0x05, 0x00, 0x14, 0x00,  // Data Header, Ring 5, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticka
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x01, 0x10,  // GEO 0, TDC 0, VMM 1, CH 16
};


std::vector<uint8_t> MonitorReadout {
  // First monitor readout - Valid
  0x16, 0x00, 0x14, 0x00,  // Data Header - Ring 22, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x00,  // 0x00000000
  0x00, 0x00, 0x00, 0x00,  // 0x00000000

  // Second monitor readout - invalid channel
  0x17, 0x00, 0x14, 0x00,  // Data Header, Ring 23, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x00, 0x00, 0x00, 0x00,  // 0x00000000
  0x00, 0x00, 0x00, 0x01,  // CH 1 is invalid

  // Third monitor readout - invalid VMM
  0x17, 0x00, 0x14, 0x00,  // Data Header, Ring 23, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x00, 0x00, 0x00, 0x00,  // 0x00000000
  0x00, 0x00, 0x01, 0x00,  // VMM 1 invalid

  // Fourth monitor readout - invalid TDC
  0x17, 0x00, 0x14, 0x00,  // Data Header, Ring 23, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x00, 0x00, 0x00, 0x00,  // 0x00000000
  0x00, 0x01, 0x00, 0x00,  // TDC 1 invalid

  // Fifth monitor readout - invalid GEO
  0x17, 0x00, 0x14, 0x00,  // Data Header, Ring 23, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x00, 0x00, 0x00, 0x00,  // 0x00000000
  0x01, 0x00, 0x00, 0x00,  // GEO 1 invalid

  // Sixth monitor readout - invalid OTADC
  0x17, 0x00, 0x14, 0x00,  // Data Header, Ring 23, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x00, 0x00, 0x01, 0x00,  // OTADC 1 invalid
  0x00, 0x00, 0x00, 0x00,  // 0x00000000

  // Seventh monitor readout - invalid BC
  0x17, 0x00, 0x14, 0x00,  // Data Header, Ring 23, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x01, 0x00, 0x00, 0x00,  // BC 1 invalid
  0x00, 0x00, 0x00, 0x00,  // 0x00000000

  // Eighth monitor readout - invalid Ring
  0x15, 0x00, 0x14, 0x00,  // Data Header - Ring 21 invalid, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x00,  // 0x00000000
  0x00, 0x00, 0x00, 0x00,  // 0x00000000

  // Ninth monitor readout - invalid FEN
  0x16, 0x01, 0x14, 0x00,  // Data Header - Ring 22, FEN 1 invalid
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x00,  // 0x00000000
  0x00, 0x00, 0x00, 0x00,  // 0x00000000

  // Tenth monitor readout - TOF too large
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


class FreiaInstrumentTest : public TestBase {
public:

protected:
  struct Counters counters;
  FreiaSettings ModuleSettings;
  EV42Serializer * serializer;
  FreiaInstrument * freia;
  ESSReadout::Parser::PacketHeaderV0 PacketHeader;
  Event TestEvent;           // used for testing generateEvents()
  std::vector<Event> Events; // used for testing generateEvents()

  void SetUp() override {
    ModuleSettings.ConfigFile = ConfigFile;
    serializer = new EV42Serializer(115000, "freia");
    counters = {};

    memset(&PacketHeader, 0, sizeof(PacketHeader));

    freia = new FreiaInstrument(counters, ModuleSettings, serializer);
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
TEST_F(FreiaInstrumentTest, Constructor) {
  ASSERT_EQ(counters.RingErrors, 0);
  ASSERT_EQ(counters.FENErrors, 0);
}


/// THIS IS NOT A TEST, just ensure we also try dumping to hdf5
TEST_F(FreiaInstrumentTest, DumpTofile) {
  ModuleSettings.FilePrefix = "deleteme_";
  FreiaInstrument FreiaDump(counters, ModuleSettings, serializer);
  FreiaDump.setSerializer(serializer);

  makeHeader(FreiaDump.ESSReadoutParser.Packet, GoodEvent);
  auto Res = FreiaDump.VMMParser.parse(FreiaDump.ESSReadoutParser.Packet);
  FreiaDump.processReadouts();

  counters.VMMStats = FreiaDump.VMMParser.Stats;
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(counters.VMMStats.Readouts, 2);
}

TEST_F(FreiaInstrumentTest, TwoReadouts) {
  makeHeader(freia->ESSReadoutParser.Packet, MappingError);
  auto Res = freia->VMMParser.parse(freia->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(counters.RingErrors, 0);
  ASSERT_EQ(counters.FENErrors, 0);

  freia->processReadouts();
  ASSERT_EQ(counters.RingErrors, 1);
  ASSERT_EQ(counters.FENErrors, 1);
}

TEST_F(FreiaInstrumentTest, WireGap) {
  TestEvent.ClusterA.insert({0, 1, 100, 0});
  TestEvent.ClusterB.insert({0, 1, 100, 1});
  TestEvent.ClusterB.insert({0, 3, 100, 1});
  Events.push_back(TestEvent);

  freia->generateEvents(Events);
  ASSERT_EQ(counters.EventsInvalidWireGap, 1);
}

TEST_F(FreiaInstrumentTest, StripGap) {
  TestEvent.ClusterA.insert({0, 1, 100, 0});
  TestEvent.ClusterA.insert({0, 3, 100, 0});
  TestEvent.ClusterB.insert({0, 1, 100, 1});
  Events.push_back(TestEvent);

  freia->generateEvents(Events);
  ASSERT_EQ(counters.EventsInvalidStripGap, 1);
}

TEST_F(FreiaInstrumentTest, ClusterWireOnly) {
  TestEvent.ClusterB.insert({0, 1, 0, 0});
  Events.push_back(TestEvent);

  freia->generateEvents(Events);
  ASSERT_EQ(counters.EventsMatchedWireOnly, 1);
}

TEST_F(FreiaInstrumentTest, ClusterStripOnly) {
  TestEvent.ClusterA.insert({0, 1, 0, 0});
  Events.push_back(TestEvent);

  freia->generateEvents(Events);
  ASSERT_EQ(counters.EventsMatchedStripOnly, 1);
}

TEST_F(FreiaInstrumentTest, PixelError) {
  makeHeader(freia->ESSReadoutParser.Packet, PixelError);
  auto Res = freia->VMMParser.parse(freia->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 2);

  freia->processReadouts();
  for (auto & builder : freia->builders) {
    builder.flush();
    freia->generateEvents(builder.Events);
  }
  ASSERT_EQ(counters.PixelErrors, 1);
}

TEST_F(FreiaInstrumentTest, EventTOFError) {
  auto & Packet = freia->ESSReadoutParser.Packet;
  makeHeader(Packet, GoodEvent);

  Packet.Time.setReference(200, 0);
  auto Res = freia->VMMParser.parse(Packet);
  counters.VMMStats = freia->VMMParser.Stats;

  freia->processReadouts();
  for (auto & builder : freia->builders) {
    builder.flush();
    freia->generateEvents(builder.Events);
  }
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(counters.VMMStats.Readouts, 2);
  ASSERT_EQ(counters.TimeErrors, 1);
}

TEST_F(FreiaInstrumentTest, GoodEvent) {
  //                         t  c  w    p
  TestEvent.ClusterA.insert({0, 3, 100, 0});
  TestEvent.ClusterB.insert({0, 1, 100, 1});
  Events.push_back(TestEvent);
  freia->generateEvents(Events);
  ASSERT_EQ(counters.Events, 1);
}

TEST_F(FreiaInstrumentTest, EventTOFTooLarge) {
  TestEvent.ClusterA.insert({3000000000, 3, 100, 0});
  TestEvent.ClusterB.insert({3000000000, 1, 100, 1});
  Events.push_back(TestEvent);
  freia->generateEvents(Events);
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.TOFErrors, 1);
}


TEST_F(FreiaInstrumentTest, NoEvents) {
  Events.push_back(TestEvent);
  freia->generateEvents(Events);
  ASSERT_EQ(counters.Events, 0);
}

TEST_F(FreiaInstrumentTest, BeamMonitor) {
  ModuleSettings.IsMonitor = true;
  makeHeader(freia->ESSReadoutParser.Packet, MonitorReadout);
  auto Readouts = freia->VMMParser.parse(freia->ESSReadoutParser.Packet);
  ASSERT_EQ(Readouts, 10);

  freia->processMonitorReadouts();
  ASSERT_EQ(counters.MonitorCounts, 1);
  ASSERT_EQ(counters.MonitorErrors, 6);
  ASSERT_EQ(counters.RingErrors, 1);
  ASSERT_EQ(counters.FENErrors, 1);
  ASSERT_EQ(counters.TOFErrors, 1);
}

TEST_F(FreiaInstrumentTest, BeamMonitorTOF) {
  ModuleSettings.IsMonitor = true;
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
