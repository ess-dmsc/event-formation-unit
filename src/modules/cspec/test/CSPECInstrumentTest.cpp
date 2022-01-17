// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/kafka/EV42Serializer.h>
#include <cspec/CSPECInstrument.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <stdio.h>
#include <string.h>

using namespace Cspec;

std::string ConfigFile{"deleteme_cspec_instr_config.json"};
std::string ConfigStr = R"(
  {
    "Detector": "CSPEC",
    "InstrumentGeometry" : "CSPEC",

    "Vessel_Config" : [
    {"ID":  0, "NumGrids": 140, "Rotation": false, "XOffset":   0}
    ],

    "Config" : [
      { "Ring" :  0, "VesselId": 0, "FEN": 0, "Hybrid" :  0, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 0, "FEN": 0, "Hybrid" :  1, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 0, "FEN": 0, "Hybrid" :  2, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 0, "FEN": 1, "Hybrid" :  0, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 0, "FEN": 1, "Hybrid" :  1, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 0, "FEN": 1, "Hybrid" :  2, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 1, "FEN": 2, "Hybrid" :  0, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 1, "FEN": 2, "Hybrid" :  1, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 1, "FEN": 2, "Hybrid" :  2, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 1, "FEN": 3, "Hybrid" :  0, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 1, "FEN": 3, "Hybrid" :  1, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 1, "FEN": 3, "Hybrid" :  2, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 2, "FEN": 4, "Hybrid" :  0, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 2, "FEN": 4, "Hybrid" :  1, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 2, "FEN": 4, "Hybrid" :  2, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 2, "FEN": 5, "Hybrid" :  0, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 2, "FEN": 5, "Hybrid" :  1, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 2, "FEN": 5, "Hybrid" :  2, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 3, "FEN": 6, "Hybrid" :  0, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 3, "FEN": 6, "Hybrid" :  1, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 3, "FEN": 6, "Hybrid" :  2, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 3, "FEN": 7, "Hybrid" :  0, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 3, "FEN": 7, "Hybrid" :  1, "HybridId" : ""},
      { "Ring" :  0, "VesselId": 3, "FEN": 7, "Hybrid" :  2, "HybridId" : ""}
    ],

    "MaxPulseTimeNS" : 71428570,
    "TimeBoxNs" : 2010
  }
)";


//
std::vector<uint8_t> MappingError {
  // First readout
  0x16, 0x01, 0x14, 0x00,  // Data Header - Ring 22!
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
  // First readout - plane Y - Wires
  0x04, 0x01, 0x14, 0x00,  // Data Header - Ring 4, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x10,  // GEO 0, TDC 0, VMM 0, CH 16

  // Second readout - plane X - Strips
  0x05, 0x01, 0x14, 0x00,  // Data Header, Ring 5, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticka
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x01, 0x10,  // GEO 0, TDC 0, VMM 1, CH 16
};


class CSPECInstrumentTest : public TestBase {
public:

protected:
  struct Counters counters;
  CSPECSettings ModuleSettings;
  EV42Serializer * serializer;
  CSPECInstrument * cspec;
  ESSReadout::Parser::PacketHeaderV0 PacketHeader;
  Event TestEvent;           // used for testing generateEvents()
  std::vector<Event> Events; // used for testing generateEvents()

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

  void makeHeader(ESSReadout::Parser::PacketDataV0 & Packet, std::vector<uint8_t> & testdata) {
    Packet.HeaderPtr = &PacketHeader;
    Packet.DataPtr = (char *)&testdata[0];
    Packet.DataLength = testdata.size();
    Packet.Time.setReference(0,0);
    Packet.Time.setPrevReference(0,0);
  }
};

// Test cases below
TEST_F(CSPECInstrumentTest, Constructor) {
  ASSERT_EQ(counters.HybridErrors, 0);
  // ASSERT_EQ(counters.FENErrors, 0);
}


int main(int argc, char **argv) {
  saveBuffer(ConfigFile, (void *)ConfigStr.c_str(), ConfigStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(ConfigFile);
  return RetVal;
}
