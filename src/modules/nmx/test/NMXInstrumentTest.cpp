// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/kafka/EV42Serializer.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <nmx/NMXInstrument.h>
#include <stdio.h>
#include <string.h>

using namespace Nmx;

// clang-format off
std::string BadConfigFile{"deleteme_nmx_instr_config_bad.json"};
std::string BadConfigStr = R"(
  {
  "NotARealDetector" : "NMX",
  "InstrumentGeometry" : "NMX",
  "MaxSpanX" : 3,
  "MaxSpanY" : 3,
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
          "ReversedChannels" : false,
          "Panel" : 0,
          "HybridId" : "E5533333222222221111111100000000"
        },
        {
          "Ring" :  0, 
          "FEN": 0, 
          "Hybrid" :  1, 
          "Plane" : 1,
          "Offset" : 0,
          "ReversedChannels" : false,
          "Panel" : 0,
          "HybridId" : "E5533333222222221111111100000001"
        }
]
}
)";


std::string ConfigFile{"deleteme_nmx_instr_config.json"};
std::string ConfigStr = R"(
{
  "Detector" : "NMX",
  "InstrumentGeometry" : "NMX",
  "MaxSpanX" : 3,
  "MaxSpanY" : 3,
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
          "ReversedChannels" : false,
          "Panel" : 0,
          "HybridId" : "E5533333222222221111111100000000"
        },
        {
          "Ring" :  0, 
          "FEN": 0, 
          "Hybrid" :  1, 
          "Plane" : 1,
          "Offset" : 0,
          "ReversedChannels" : false,
          "Panel" : 0,
          "HybridId" : "E5533333222222221111111100000001"
        }
]
}
)";


//
std::vector<uint8_t> BadRingAndFENError {
  // First readout
  0x18, 0x01, 0x14, 0x00,  // Data Header - Ring 23!
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x10,  // GEO 0, TDC 0, VMM 0, CH 16

  // Second readout
  0x02, 0x18, 0x14, 0x00,  // Data Header - Ring 2, FEN 3
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticka
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x01, 0x10,  // GEO 0, TDC 0, VMM 1, CH 16
};



std::vector<uint8_t> GoodEvent {
  // First readout - plane Y 
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60

  // Second readout - plane Y 
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x02, 0x00, 0x00, 0x00,  // Time LO 2 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3D,  // GEO 0, TDC 0, VMM 0, CH 61

  // Third readout - plane X 
  0x00, 0x00, 0x14, 0x00,  // Data Header, Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3C,  // GEO 0, TDC 0, VMM 1, CH 60

};


std::vector<uint8_t> SplitEventA {
  // First readout - plane Y 
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3C,  // GEO 0, TDC 0, VMM 1, CH 60

};

std::vector<uint8_t> SplitEventB {
  // Second readout - plane Y
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x02, 0x00, 0x00, 0x00,  // Time LO 2 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3D,  // GEO 0, TDC 0, VMM 1, CH 61

  // Third readout - plane X
  0x00, 0x00, 0x14, 0x00,  // Data Header, Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60

};


std::vector<uint8_t> HighTOFError {
  // First readout - plane Y
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3C,  // GEO 0, TDC 0, VMM 1, CH 60

  // Second readout - plane Y
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x02, 0x00, 0x00, 0x00,  // Time LO 2 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3D,  // GEO 0, TDC 0, VMM 1, CH 61

  // Third readout - plane X
  0x00, 0x00, 0x14, 0x00,  // Data Header, Ring 0, FEN 0
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60
};


std::vector<uint8_t> BadEventLargeYSpan {
  // First readout - plane Y 
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3C,  // GEO 0, TDC 0, VMM 1, CH 60

  // Second readout - plane Y 
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x02, 0x00, 0x00, 0x00,  // Time LO 2 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3D,  // GEO 0, TDC 0, VMM 1, CH 61

  // Second readout - plane Y 
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x02, 0x00, 0x00, 0x00,  // Time LO 2 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3E,  // GEO 0, TDC 0, VMM 1, CH 62

  // Second readout - plane Y 
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x02, 0x00, 0x00, 0x00,  // Time LO 2 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3F,  // GEO 0, TDC 0, VMM 1, CH 63

  // Third readout - plane X
  0x00, 0x00, 0x14, 0x00,  // Data Header, Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60
};


std::vector<uint8_t> BadEventLargeTimeSpan {
  // First readout - plane Y
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3C,  // GEO 0, TDC 0, VMM 1, CH 60

  // Second readout - plane Y 
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0xFF, 0x04, 0x00, 0x00,  // Time LO 500 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3D,  // GEO 0, TDC 0, VMM 1, CH 61

  // Second readout - plane Y 
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x14, 0x00, 0x00, 0x00,  // Time LO 20 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3E,  // GEO 0, TDC 0, VMM 1, CH 62

  // Third readout - plane X 
  0x00, 0x00, 0x14, 0x00,  // Data Header, Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60
};


std::vector<uint8_t> MaxADC {
  // First readout - plane Y
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 256
  0x00, 0x00, 0x02, 0x3C,  // GEO 0, TDC 0, VMM 1, CH 60

  // Second readout - plane X
  0x00, 0x00, 0x14, 0x00,  // Data Header, Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0xD0, 0x07,  // ADC = 2000, above threshold
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60
};

std::vector<uint8_t> MinADC {
  // First readout - plane X 
  0x00, 0x00, 0x14, 0x00,  // Data Header, Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0x28, 0x00,  // ADC = 40, under default threshold required
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60
 
  // Second readout - plane X 
  0x00, 0x00, 0x14, 0x00,  // Data Header, Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0x4B, 0x00,  // ADC = 75, over threshold required
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60

  // Third readout - plane X 
  0x00, 0x00, 0x14, 0x00,  // Data Header, Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0x28, 0x00,  // ADC = 40, under default threshold required
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60
};

std::vector<uint8_t> NoEventYOnly {
  // First readout - plane Y
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3C,  // GEO 0, TDC 0, VMM 1, CH 60

  // Second readout - plane Y
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3D,  // GEO 0, TDC 0, VMM 1, CH 61
};

std::vector<uint8_t> NoEventXOnly {
  // First readout - plane X 
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60

  // Second readout - plane X
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3D,  // GEO 0, TDC 0, VMM 0, CH 61
};

// clang-format on

class NMXInstrumentTest : public TestBase {
public:
protected:
  struct Counters counters;
  NMXSettings ModuleSettings;
  EV42Serializer *serializer;
  NMXInstrument *nmx;
  ESSReadout::Parser::PacketHeaderV0 PacketHeader;
  Event TestEvent;           // used for testing generateEvents()
  std::vector<Event> Events; // used for testing generateEvents()

  void SetUp() override {
    ModuleSettings.ConfigFile = ConfigFile;
    serializer = new EV42Serializer(115000, "nmx");
    counters = {};

    memset(&PacketHeader, 0, sizeof(PacketHeader));

    nmx = new NMXInstrument(counters, ModuleSettings, serializer);
    nmx->setSerializer(serializer);
    nmx->ESSReadoutParser.Packet.HeaderPtr = &PacketHeader;
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

/// THIS IS NOT A TEST, just ensure we also try dumping to hdf5
TEST_F(NMXInstrumentTest, DumpTofile) {
  ModuleSettings.FilePrefix = "deleteme_";
  NMXInstrument NMXDump(counters, ModuleSettings, serializer);
  NMXDump.setSerializer(serializer);

  makeHeader(NMXDump.ESSReadoutParser.Packet, GoodEvent);
  auto Res = NMXDump.VMMParser.parse(NMXDump.ESSReadoutParser.Packet);
  NMXDump.processReadouts();

  counters.VMMStats = NMXDump.VMMParser.Stats;
  ASSERT_EQ(Res, 3);
  ASSERT_EQ(counters.VMMStats.Readouts, 3);
}

// Test cases below
TEST_F(NMXInstrumentTest, BadConfig) {
  ModuleSettings.ConfigFile = BadConfigFile;
  EXPECT_THROW(NMXInstrument(counters, ModuleSettings, serializer),
               std::runtime_error);
}

TEST_F(NMXInstrumentTest, Constructor) {
  ASSERT_EQ(counters.HybridMappingErrors, 0);
}

TEST_F(NMXInstrumentTest, BadRingAndFENError) {
  makeHeader(nmx->ESSReadoutParser.Packet, BadRingAndFENError);
  auto Res = nmx->VMMParser.parse(nmx->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 0);
  counters.VMMStats = nmx->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.ErrorRing, 1);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 1);
}

TEST_F(NMXInstrumentTest, GoodEvent) {
  makeHeader(nmx->ESSReadoutParser.Packet, GoodEvent);
  auto Res = nmx->VMMParser.parse(nmx->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 3);
  counters.VMMStats = nmx->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.ErrorRing, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);

  // Ring and FEN IDs are within bounds, but Hybrid is not defined in config
  nmx->processReadouts();
  ASSERT_EQ(counters.VMMStats.ErrorRing, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);
  ASSERT_EQ(counters.VMMStats.Readouts, 3);

  for (auto &builder : nmx->builders) {
    builder.flush(true);
    nmx->generateEvents(builder.Events);
  }
  ASSERT_EQ(counters.Events, 1);
}

TEST_F(NMXInstrumentTest, MaxADC) {
  makeHeader(nmx->ESSReadoutParser.Packet, MaxADC);
  auto Res = nmx->VMMParser.parse(nmx->ESSReadoutParser.Packet);
  counters.VMMStats = nmx->VMMParser.Stats;

  // ADC was above VMM threshold of 1023 once
  ASSERT_EQ(counters.VMMStats.ErrorADC, 1);
  ASSERT_EQ(Res, 1);
}

TEST_F(NMXInstrumentTest, MinADC) {
  makeHeader(nmx->ESSReadoutParser.Packet, MinADC);
  auto Res = nmx->VMMParser.parse(nmx->ESSReadoutParser.Packet);
  counters.VMMStats = nmx->VMMParser.Stats;
  ASSERT_EQ(Res, 3);
  ASSERT_EQ(counters.VMMStats.ErrorADC, 0);

  nmx->processReadouts();
  ASSERT_EQ(counters.MinADC, 2); // ADC was under vessel specific threshold
                                 // once, under general default once
}

TEST_F(NMXInstrumentTest, NoEventYOnly) {
  makeHeader(nmx->ESSReadoutParser.Packet, NoEventYOnly);
  auto Res = nmx->VMMParser.parse(nmx->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 2);
  counters.VMMStats = nmx->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.ErrorRing, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);

  nmx->processReadouts();
  ASSERT_EQ(counters.VMMStats.ErrorRing, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);
  counters.VMMStats = nmx->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.Readouts, 2);

  for (auto &builder : nmx->builders) {
    builder.flush(true);
    nmx->generateEvents(builder.Events);
  }
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.ClustersNoCoincidence, 1);
  ASSERT_EQ(counters.ClustersMatchedYOnly, 1);
}

TEST_F(NMXInstrumentTest, NoEventXOnly) {
  makeHeader(nmx->ESSReadoutParser.Packet, NoEventXOnly);
  auto Res = nmx->VMMParser.parse(nmx->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 2);
  counters.VMMStats = nmx->VMMParser.Stats;

  ASSERT_EQ(counters.VMMStats.ErrorRing, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);

  nmx->processReadouts();
  ASSERT_EQ(counters.VMMStats.ErrorRing, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);
  ASSERT_EQ(counters.VMMStats.Readouts, 2);

  for (auto &builder : nmx->builders) {
    builder.flush(true);
    nmx->generateEvents(builder.Events);
  }
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.ClustersNoCoincidence, 1);
  ASSERT_EQ(counters.ClustersMatchedXOnly, 1);
}

TEST_F(NMXInstrumentTest, NoEvents) {
  Events.push_back(TestEvent);
  nmx->generateEvents(Events);
  ASSERT_EQ(counters.Events, 0);
}

TEST_F(NMXInstrumentTest, PixelError) {
  TestEvent.ClusterA.insert({0, 1, 100, 0});
  TestEvent.ClusterB.insert({0, 60000, 100, 1});
  Events.push_back(TestEvent);
  nmx->generateEvents(Events);
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.PixelErrors, 1);
}

TEST_F(NMXInstrumentTest, BadEventLargeYSpan) {
  makeHeader(nmx->ESSReadoutParser.Packet, BadEventLargeYSpan);
  auto Res = nmx->VMMParser.parse(nmx->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 5);
  counters.VMMStats = nmx->VMMParser.Stats;

  ASSERT_EQ(counters.VMMStats.ErrorRing, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);

  nmx->processReadouts();
  ASSERT_EQ(counters.VMMStats.ErrorRing, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);
  ASSERT_EQ(counters.VMMStats.Readouts, 5);

  for (auto &builder : nmx->builders) {
    builder.flush(true);
    nmx->generateEvents(builder.Events);
  }
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.ClustersTooLargeYSpan, 1);
}

TEST_F(NMXInstrumentTest, NegativeTOF) {
  auto &Packet = nmx->ESSReadoutParser.Packet;
  makeHeader(nmx->ESSReadoutParser.Packet, GoodEvent);
  Packet.Time.setReference(200, 0);

  auto Res = nmx->VMMParser.parse(nmx->ESSReadoutParser.Packet);
  counters.VMMStats = nmx->VMMParser.Stats;

  nmx->processReadouts();
  for (auto &builder : nmx->builders) {
    builder.flush(true);
    nmx->generateEvents(builder.Events);
  }

  ASSERT_EQ(Res, 3);
  ASSERT_EQ(counters.VMMStats.Readouts, 3);
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.TimeErrors, 1);
}

TEST_F(NMXInstrumentTest, HighTOFError) {
  makeHeader(nmx->ESSReadoutParser.Packet, HighTOFError);

  auto Res = nmx->VMMParser.parse(nmx->ESSReadoutParser.Packet);
  counters.VMMStats = nmx->VMMParser.Stats;

  nmx->processReadouts();
  for (auto &builder : nmx->builders) {
    builder.flush(true);
    nmx->generateEvents(builder.Events);
  }

  ASSERT_EQ(Res, 3);
  ASSERT_EQ(counters.VMMStats.Readouts, 3);
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.TOFErrors, 1);
}

TEST_F(NMXInstrumentTest, BadEventLargeTimeSpan) {
  makeHeader(nmx->ESSReadoutParser.Packet, BadEventLargeTimeSpan);

  auto Res = nmx->VMMParser.parse(nmx->ESSReadoutParser.Packet);
  counters.VMMStats = nmx->VMMParser.Stats;

  nmx->processReadouts();
  for (auto &builder : nmx->builders) {
    builder.flush(true);
    nmx->generateEvents(builder.Events);
  }

  ASSERT_EQ(Res, 4);
  ASSERT_EQ(counters.VMMStats.Readouts, 4);
  ASSERT_EQ(counters.Events, 1);
  ASSERT_EQ(counters.ClustersNoCoincidence, 1);
}

TEST_F(NMXInstrumentTest, EventCrossPackets) {
  makeHeader(nmx->ESSReadoutParser.Packet, SplitEventA);

  auto Res = nmx->VMMParser.parse(nmx->ESSReadoutParser.Packet);
  counters.VMMStats = nmx->VMMParser.Stats;

  nmx->processReadouts();
  for (auto &builder : nmx->builders) {
    builder.flush();
    nmx->generateEvents(builder.Events);
  }
  ASSERT_EQ(Res, 1);

  makeHeader(nmx->ESSReadoutParser.Packet, SplitEventB);

  Res = nmx->VMMParser.parse(nmx->ESSReadoutParser.Packet);
  counters.VMMStats = nmx->VMMParser.Stats;

  nmx->processReadouts();
  for (auto &builder : nmx->builders) {
    builder.flush(true);
    nmx->generateEvents(builder.Events);
  }

  ASSERT_EQ(Res, 2);
  ASSERT_EQ(counters.VMMStats.Readouts, 3);
  ASSERT_EQ(counters.Events, 1);
}

int main(int argc, char **argv) {
  saveBuffer(ConfigFile, (void *)ConfigStr.c_str(), ConfigStr.size());
  saveBuffer(BadConfigFile, (void *)BadConfigStr.c_str(), BadConfigStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(ConfigFile);
  deleteFile(BadConfigFile);
  return RetVal;
}
