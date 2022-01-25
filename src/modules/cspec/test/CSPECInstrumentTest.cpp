// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/kafka/EV42Serializer.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <cspec/CSPECInstrument.h>
#include <stdio.h>
#include <string.h>

using namespace Cspec;

// clang-format off
std::string ConfigFile{"deleteme_cspec_instr_config.json"};
std::string ConfigStr = R"(
  {
    "Detector": "CSPEC",
    "InstrumentGeometry" : "CSPEC",

    "Vessel_Config" : {
      "0": {"NumGrids": 140, "Rotation": false, "XOffset":   0},
      "1": {"NumGrids": 140, "Rotation": false, "XOffset":  12},
      "2": {"NumGrids": 140, "Rotation": false, "XOffset":  24, "MinADC": 100},
      "3": {"NumGrids": 140, "Rotation": false, "XOffset":  36}
    },

    "Config" : [
      { "Ring" :  0, "VesselId": "0", "FEN": 0, "Hybrid" :  1, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "0", "FEN": 0, "Hybrid" :  2, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "0", "FEN": 1, "Hybrid" :  0, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "0", "FEN": 1, "Hybrid" :  1, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "0", "FEN": 1, "Hybrid" :  2, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "1", "FEN": 2, "Hybrid" :  0, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "1", "FEN": 2, "Hybrid" :  1, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "1", "FEN": 2, "Hybrid" :  2, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "1", "FEN": 3, "Hybrid" :  0, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "1", "FEN": 3, "Hybrid" :  1, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "1", "FEN": 3, "Hybrid" :  2, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "2", "FEN": 4, "Hybrid" :  0, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "2", "FEN": 4, "Hybrid" :  1, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "2", "FEN": 4, "Hybrid" :  2, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "2", "FEN": 5, "Hybrid" :  0, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "2", "FEN": 5, "Hybrid" :  1, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "2", "FEN": 5, "Hybrid" :  2, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "3", "FEN": 6, "Hybrid" :  0, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "3", "FEN": 6, "Hybrid" :  1, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "3", "FEN": 6, "Hybrid" :  2, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "3", "FEN": 7, "Hybrid" :  0, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "3", "FEN": 7, "Hybrid" :  1, "HybridId" : ""},
      { "Ring" :  0, "VesselId": "3", "FEN": 7, "Hybrid" :  2, "HybridId" : ""}
    ],

    "MaxPulseTimeNS" : 71428570,
    "TimeBoxNs" : 2010
  }
)";


//
std::vector<uint8_t> BadRingAndFENError {
  // First readout
  0x16, 0x01, 0x14, 0x00,  // Data Header - Ring 22!
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x10,  // GEO 0, TDC 0, VMM 0, CH 16

  // Second readout
  0x02, 0x14, 0x14, 0x00,  // Data Header - Ring 2, FEN 3
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticka
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x01, 0x10,  // GEO 0, TDC 0, VMM 1, CH 16
};

std::vector<uint8_t> BadHybridError {
  // First readout, in test config Ring 0, FEN 0, Hybrid 0 is undefined
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x10,  // GEO 0, TDC 0, VMM 0, CH 16
};

std::vector<uint8_t> PixelError {
  // First readout - plane Y - Wires
  0x04, 0x01, 0x14, 0x00,  // Data Header - Ring 4, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x32,  // GEO 0, TDC 0, VMM 0, CH 50

  // Second readout - plane X - Strips
  0x05, 0x01, 0x14, 0x00,  // Data Header, Ring 5, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticka
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x01, 0x10,  // GEO 0, TDC 0, VMM 1, CH 16
};

std::vector<uint8_t> GoodEvent {
  // First readout - plane Y - Grids
  0x00, 0x01, 0x14, 0x00,  // Data Header - Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3C,  // GEO 0, TDC 0, VMM 1, CH 60

  // Second readout - plane Y - Grids
  0x00, 0x01, 0x14, 0x00,  // Data Header - Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x02, 0x00, 0x00, 0x00,  // Time LO 2 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3D,  // GEO 0, TDC 0, VMM 1, CH 61

  // Third readout - plane X & Z - Wires
  0x00, 0x01, 0x14, 0x00,  // Data Header, Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60
};


std::vector<uint8_t> BadEventLargeGridSpan {
  // First readout - plane Y - Grids
  0x00, 0x01, 0x14, 0x00,  // Data Header - Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3C,  // GEO 0, TDC 0, VMM 1, CH 60

  // Second readout - plane Y - Grids
  0x00, 0x01, 0x14, 0x00,  // Data Header - Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x02, 0x00, 0x00, 0x00,  // Time LO 2 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3D,  // GEO 0, TDC 0, VMM 1, CH 61

  // Second readout - plane Y - Grids
  0x00, 0x01, 0x14, 0x00,  // Data Header - Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x02, 0x00, 0x00, 0x00,  // Time LO 2 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3E,  // GEO 0, TDC 0, VMM 1, CH 62

  // Second readout - plane Y - Grids
  0x00, 0x01, 0x14, 0x00,  // Data Header - Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x02, 0x00, 0x00, 0x00,  // Time LO 2 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3F,  // GEO 0, TDC 0, VMM 1, CH 63

  // Third readout - plane X & Z - Wires
  0x00, 0x01, 0x14, 0x00,  // Data Header, Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60
};



std::vector<uint8_t> BadMappingError {
  // First readout - plane Y - Grids
  0x00, 0x01, 0x14, 0x00,  // Data Header - Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x05,  // GEO 0, TDC 0, VMM 2, CH 5

  // Second readout - plane X & Z - Wires
  0x00, 0x01, 0x14, 0x00,  // Data Header, Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x05,  // GEO 0, TDC 0, VMM 0, CH 5
};

std::vector<uint8_t> MaxADC {
  // First readout - plane Y - Grids
  0x00, 0x01, 0x14, 0x00,  // Data Header - Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 256
  0x00, 0x00, 0x02, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60

  // Second readout - plane X & Z - Wires
  0x00, 0x01, 0x14, 0x00,  // Data Header, Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0xD0, 0x07,  // ADC = 2000, above threshold
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60
};

std::vector<uint8_t> MinADC {
 // First readout - plane X & Z - Wires
  0x00, 0x01, 0x14, 0x00,  // Data Header, Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0x28, 0x00,  // ADC = 40, under default threshold required
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60
 
 // Third readout - plane X & Z - Wires
  0x00, 0x01, 0x14, 0x00,  // Data Header, Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0x4B, 0x00,  // ADC = 75, over threshold required
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60

   // Fourth readout - plane X & Z - Wires
  0x00, 0x04, 0x14, 0x00,  // Data Header, Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0x4B, 0x00,  // ADC = 75, under threshold for this specific vessel
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60
};

std::vector<uint8_t> NoEvent {
  // First readout - plane Y - Grids
  0x00, 0x01, 0x14, 0x00,  // Data Header - Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60

  // Second readout - plane Y - Grids
  0x00, 0x01, 0x14, 0x00,  // Data Header - Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3D,  // GEO 0, TDC 0, VMM 0, CH 61
};

// clang-format on

class CSPECInstrumentTest : public TestBase {
 public:
 protected:
  struct Counters counters;
  CSPECSettings ModuleSettings;
  EV42Serializer *serializer;
  CSPECInstrument *cspec;
  ESSReadout::Parser::PacketHeaderV0 PacketHeader;
  Event TestEvent;            // used for testing generateEvents()
  std::vector<Event> Events;  // used for testing generateEvents()

  void SetUp() override {
    ModuleSettings.ConfigFile = ConfigFile;
    serializer = new EV42Serializer(115000, "cspec");
    counters = {};

    memset(&PacketHeader, 0, sizeof(PacketHeader));

    cspec = new CSPECInstrument(counters, ModuleSettings, serializer);
    cspec->setSerializer(serializer);
    cspec->ESSReadoutParser.Packet.HeaderPtr = &PacketHeader;
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
TEST_F(CSPECInstrumentTest, Constructor) {
  ASSERT_EQ(counters.HybridErrors, 0);
  ASSERT_EQ(counters.FENErrors, 0);
  ASSERT_EQ(counters.RingErrors, 0);
}

TEST_F(CSPECInstrumentTest, BadRingAndFENError) {
  makeHeader(cspec->ESSReadoutParser.Packet, BadRingAndFENError);
  auto Res = cspec->VMMParser.parse(cspec->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(counters.RingErrors, 0);
  ASSERT_EQ(counters.FENErrors, 0);

  cspec->processReadouts();
  ASSERT_EQ(counters.RingErrors, 1);
  ASSERT_EQ(counters.FENErrors, 1);
}

TEST_F(CSPECInstrumentTest, BadHybridError) {
  makeHeader(cspec->ESSReadoutParser.Packet, BadHybridError);
  auto Res = cspec->VMMParser.parse(cspec->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 1);
  ASSERT_EQ(counters.RingErrors, 0);
  ASSERT_EQ(counters.FENErrors, 0);
  ASSERT_EQ(counters.HybridErrors, 0);
  counters.VMMStats = cspec->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.Readouts, 1);

  // Ring and FEN IDs are within bounds, but Hybrid is not defined in config
  cspec->processReadouts();
  ASSERT_EQ(counters.RingErrors, 0);
  ASSERT_EQ(counters.FENErrors, 0);
  ASSERT_EQ(counters.HybridErrors, 1);
}

TEST_F(CSPECInstrumentTest, GoodEvent) {
  makeHeader(cspec->ESSReadoutParser.Packet, GoodEvent);
  auto Res = cspec->VMMParser.parse(cspec->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 3);
  ASSERT_EQ(counters.RingErrors, 0);
  ASSERT_EQ(counters.FENErrors, 0);
  ASSERT_EQ(counters.HybridErrors, 0);

  // Ring and FEN IDs are within bounds, but Hybrid is not defined in config
  cspec->processReadouts();
  ASSERT_EQ(counters.RingErrors, 0);
  ASSERT_EQ(counters.FENErrors, 0);
  ASSERT_EQ(counters.HybridErrors, 0);
  counters.VMMStats = cspec->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.Readouts, 3);

  for (auto &builder : cspec->builders) {
    builder.flush();
    cspec->generateEvents(builder.Events);
  }
  ASSERT_EQ(counters.Events, 1);
}

TEST_F(CSPECInstrumentTest, BadMappingError) {
  makeHeader(cspec->ESSReadoutParser.Packet, BadMappingError);
  auto Res = cspec->VMMParser.parse(cspec->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(counters.RingErrors, 0);
  ASSERT_EQ(counters.FENErrors, 0);
  ASSERT_EQ(counters.HybridErrors, 0);

  cspec->processReadouts();
  ASSERT_EQ(counters.RingErrors, 0);
  ASSERT_EQ(counters.FENErrors, 0);
  ASSERT_EQ(counters.HybridErrors, 0);
  counters.VMMStats = cspec->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.Readouts, 2);

  for (auto &builder : cspec->builders) {
    builder.flush();
    cspec->generateEvents(builder.Events);
  }
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.MappingErrors, 2);
}

TEST_F(CSPECInstrumentTest, MaxADC) {
  makeHeader(cspec->ESSReadoutParser.Packet, MaxADC);
  auto Res = cspec->VMMParser.parse(cspec->ESSReadoutParser.Packet);
  counters.VMMStats = cspec->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.ErrorADC,
            1);  // ADC was above VMM threshold of 1023 once
  ASSERT_EQ(Res, 1);
}

TEST_F(CSPECInstrumentTest, MinADC) {
  makeHeader(cspec->ESSReadoutParser.Packet, MinADC);
  auto Res = cspec->VMMParser.parse(cspec->ESSReadoutParser.Packet);
  counters.VMMStats = cspec->VMMParser.Stats;
  ASSERT_EQ(Res, 3);
  ASSERT_EQ(counters.VMMStats.ErrorADC, 0);

  cspec->processReadouts();
  ASSERT_EQ(counters.MinADC, 2);  // ADC was under vessel specific threshold
                                  // once, under general default once
}

TEST_F(CSPECInstrumentTest, NoEvent) {
  makeHeader(cspec->ESSReadoutParser.Packet, NoEvent);
  auto Res = cspec->VMMParser.parse(cspec->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(counters.RingErrors, 0);
  ASSERT_EQ(counters.FENErrors, 0);
  ASSERT_EQ(counters.HybridErrors, 0);

  cspec->processReadouts();
  ASSERT_EQ(counters.RingErrors, 0);
  ASSERT_EQ(counters.FENErrors, 0);
  ASSERT_EQ(counters.HybridErrors, 0);
  counters.VMMStats = cspec->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.Readouts, 2);

  for (auto &builder : cspec->builders) {
    builder.flush();
    cspec->generateEvents(builder.Events);
  }
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.EventsNoCoincidence, 1);
  ASSERT_EQ(counters.EventsMatchedGridOnly, 1);
}

TEST_F(CSPECInstrumentTest, BadEventLargeGridSpan) {
  makeHeader(cspec->ESSReadoutParser.Packet, BadEventLargeGridSpan);
  auto Res = cspec->VMMParser.parse(cspec->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 5);
  ASSERT_EQ(counters.RingErrors, 0);
  ASSERT_EQ(counters.FENErrors, 0);
  ASSERT_EQ(counters.HybridErrors, 0);

  cspec->processReadouts();
  ASSERT_EQ(counters.RingErrors, 0);
  ASSERT_EQ(counters.FENErrors, 0);
  ASSERT_EQ(counters.HybridErrors, 0);
  counters.VMMStats = cspec->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.Readouts, 5);

  for (auto &builder : cspec->builders) {
    builder.flush();
    cspec->generateEvents(builder.Events);
  }
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.EventsTooLargeGridSpan, 1);
}

int main(int argc, char **argv) {
  saveBuffer(ConfigFile, (void *)ConfigStr.c_str(), ConfigStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(ConfigFile);
  return RetVal;
}
