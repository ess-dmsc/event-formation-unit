// Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/kafka/EV42Serializer.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <freia/FreiaInstrument.h>
#include <stdio.h>
#include <string.h>

using namespace Freia;

std::string ConfigFile{"deleteme_freia_instr_config.json"};
std::string ConfigStr = R"(
  {
    "Detector": "Freia",

    "WireChOffset" : 16,

    "Config" : [
    { "CassetteNumber":  0, "Ring" :  0, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000000"},
    { "CassetteNumber":  1, "Ring" :  0, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000001"},
    { "CassetteNumber":  2, "Ring" :  0, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000002"},
    { "CassetteNumber":  3, "Ring" :  0, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000003"},
    { "CassetteNumber":  4, "Ring" :  1, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000004"},
    { "CassetteNumber":  5, "Ring" :  1, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000005"},
    { "CassetteNumber":  6, "Ring" :  1, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000006"},
    { "CassetteNumber":  7, "Ring" :  1, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000007"},
    { "CassetteNumber":  8, "Ring" :  2, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000008"},
    { "CassetteNumber":  9, "Ring" :  2, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000009"},
    { "CassetteNumber": 10, "Ring" :  2, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000010"},
    { "CassetteNumber": 11, "Ring" :  2, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000011"},
    { "CassetteNumber": 12, "Ring" :  3, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000012"},
    { "CassetteNumber": 13, "Ring" :  3, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000013"},
    { "CassetteNumber": 14, "Ring" :  4, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000014"},
    { "CassetteNumber": 15, "Ring" :  4, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000015"},
    { "CassetteNumber": 16, "Ring" :  5, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000016"},
    { "CassetteNumber": 17, "Ring" :  5, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000017"},
    { "CassetteNumber": 18, "Ring" :  6, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000018"},
    { "CassetteNumber": 19, "Ring" :  6, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000019"},
    { "CassetteNumber": 20, "Ring" :  7, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000020"},
    { "CassetteNumber": 21, "Ring" :  7, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000021"},
    { "CassetteNumber": 22, "Ring" :  8, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000022"},
    { "CassetteNumber": 23, "Ring" :  8, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000023"},
    { "CassetteNumber": 24, "Ring" :  9, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000024"},
    { "CassetteNumber": 25, "Ring" :  9, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000025"},
    { "CassetteNumber": 26, "Ring" :  9, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000026"},
    { "CassetteNumber": 27, "Ring" :  9, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000027"},
    { "CassetteNumber": 28, "Ring" : 10, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000028"},
    { "CassetteNumber": 29, "Ring" : 10, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000029"},
    { "CassetteNumber": 30, "Ring" : 10, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000030"},
    { "CassetteNumber": 31, "Ring" : 10, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000031"}
  ],

    "MaxPulseTimeNS" : 71428570,
    "MaxGapWire"  : 0,
    "MaxGapStrip" : 0,
    "TimeBoxNs" : 2010
  }
)";

std::string CalibFile{"deleteme_freia_instr_calib.json"};
std::string CalibStr = R"(
  {
  "Detector" : "Freia",
  "Version"  : 1,
  "Hybrids" : 32,
  "Comment" : "Artificial calibration file - unity (slope 1, offset 0) ",

  "Calibrations" : [
    { "VMMHybridCalibration" : {
        "HybridIndex" : 0,
        "HybridId" : "E5533333222222221111111100000000",
        "CalibrationDate" : "20210222-124533",

        "vmm0" : {
          "adc_offset" : [50.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
          "adc_slope"  : [5.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0],
          "tdc_offset" : [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
          "tdc_slope"  : [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0],
          "tdc_ofs_corr" : [0.0, 1.0]
        },
        "vmm1" : {
          "adc_offset" : [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
          "adc_slope"  : [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0],
          "tdc_offset" : [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
          "tdc_slope"  : [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0],
          "tdc_ofs_corr" : [0.0, 1.0]
        }
      }
    }
    ]
  }
)";

//

std::vector<uint8_t> MaxRingMaxFENErrors{
    // First readout
    0x18, 0x00, 0x14, 0x00, // Data Header - Ring 24 is greater than max ring number
    0x00, 0x00, 0x00, 0x00, // Time HI 0 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
    0x00, 0x00, 0x00, 0x10, // GEO 0, TDC 0, VMM 0, CH 16

    // Second readout
    0x02, 0x18, 0x14, 0x00, // Data Header - FEN 24 is greater than max FEN number
    0x00, 0x00, 0x00, 0x00, // Time HI 0 s
    0x11, 0x00, 0x00, 0x00, // Time LO 17 ticka
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
    0x00, 0x00, 0x01, 0x10, // GEO 0, TDC 0, VMM 1, CH 16
};

std::vector<uint8_t> MappingError{
    // First readout
    0x00, 0x00, 0x14, 0x00, // Data Header - Ring 24 is greater than max ring number
    0x00, 0x00, 0x00, 0x00, // Time HI 0 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
    0x00, 0x00, 0x04, 0x10, // GEO 0, TDC 0, VMM 2, CH 16

    // Second readout
    0x01, 0x00, 0x14, 0x00, // Data Header - FEN 10 is greater than max FEN number
    0x00, 0x00, 0x00, 0x00, // Time HI 0 s
    0x01, 0x00, 0x00, 0x00, // Time LO 17 ticka
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
    0x00, 0x00, 0x04, 0x10, // GEO 0, TDC 0, VMM 2, CH 16
};

std::vector<uint8_t> GoodEvent{
    // First readout - plane Y - Wires
    0x04, 0x00, 0x14, 0x00, // Data Header - Ring 4, FEN 0
    0x00, 0x00, 0x00, 0x00, // Time HI 0 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
    0x00, 0x00, 0x00, 0x10, // GEO 0, TDC 0, VMM 0, CH 16

    // Second readout - plane X - Strips
    0x05, 0x00, 0x14, 0x00, // Data Header, Ring 5, FEN 0
    0x00, 0x00, 0x00, 0x00, // Time HI 0 s
    0x11, 0x00, 0x00, 0x00, // Time LO 17 ticka
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
    0x00, 0x00, 0x01, 0x10, // GEO 0, TDC 0, VMM 1, CH 16
};

class FreiaInstrumentTest : public TestBase {
public:
protected:
  struct Counters counters;
  FreiaSettings ModuleSettings;
  EV42Serializer *serializer;
  FreiaInstrument *freia;
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
TEST_F(FreiaInstrumentTest, Constructor) {
  ModuleSettings.CalibFile = CalibFile;
  FreiaInstrument Freia(counters, ModuleSettings, serializer);
  counters.VMMStats = freia->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.ErrorRing, 0);
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

TEST_F(FreiaInstrumentTest, MappingError) {
  makeHeader(freia->ESSReadoutParser.Packet, MappingError);
  auto Res = freia->VMMParser.parse(freia->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 2);
  counters.VMMStats = freia->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.ErrorRing, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);


  freia->processReadouts();
  ASSERT_EQ(counters.HybridMappingErrors, 2);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.VMMStats.ErrorRing, 0);
}

TEST_F(FreiaInstrumentTest, MaxRingMaxFENErrors) {
  makeHeader(freia->ESSReadoutParser.Packet, MaxRingMaxFENErrors);
  auto Res = freia->VMMParser.parse(freia->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 0);
  counters.VMMStats = freia->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.ErrorRing, 1);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 1);
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
  //                         t  c  w    p
  TestEvent.ClusterA.insert({0, 2000, 100, 0});
  TestEvent.ClusterB.insert({0, 1, 100, 1});
  Events.push_back(TestEvent);
  freia->generateEvents(Events);
  ASSERT_EQ(counters.PixelErrors, 1);
}

TEST_F(FreiaInstrumentTest, EventTOFError) {
  auto &Packet = freia->ESSReadoutParser.Packet;
  makeHeader(Packet, GoodEvent);

  Packet.Time.setReference(200, 0);
  auto Res = freia->VMMParser.parse(Packet);
  counters.VMMStats = freia->VMMParser.Stats;

  freia->processReadouts();
  for (auto &builder : freia->builders) {
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

int main(int argc, char **argv) {
  saveBuffer(ConfigFile, (void *)ConfigStr.c_str(), ConfigStr.size());
  saveBuffer(CalibFile, (void *)CalibStr.c_str(), CalibStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(ConfigFile);
  deleteFile(CalibFile);
  return RetVal;
}
