// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/kafka/EV44Serializer.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <trex/TREXInstrument.h>
#include <stdio.h>
#include <string.h>

using namespace Trex;

// clang-format off
std::string BadConfigFile{"deleteme_trex_instr_config_bad.json"};
std::string BadConfigStr = R"(
  {
    "Detector": "TREX",
    "InstrumentGeometry" : "Not_TREX",

    "Vessel_Config" : {
      "0": {"NumGrids": 140, "Rotation": false, "XOffset":   0}
    },

    "Config" : [
      { "Ring" :  0, "VesselId": "0", "FEN": 0, "Hybrid" :  1, "HybridId" : ""}
    ],

    "MaxPulseTimeNS" : 71428570
  }
)";


std::string ConfigFile{"deleteme_trex_instr_config.json"};
std::string ConfigStr = R"(
  {
    "Detector": "TREX",
    "InstrumentGeometry" : "TREX",

    "Vessel_Config" : {
      "0": {"NumGrids": 140, "Rotation": false, "XOffset":   0},
      "1": {"NumGrids": 140, "Rotation": false, "XOffset":  12},
      "2": {"NumGrids": 140, "Rotation": false, "XOffset":  24, "MinADC": 100},
      "3": {"NumGrids": 140, "Rotation": false, "XOffset":  36}
    },

    "Config" : [
      { "Ring" :  0, "VesselId": "0", "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000001"},
      { "Ring" :  0, "VesselId": "0", "FEN": 0, "Hybrid" :  2, "HybridId" : "E5533333222222221111111100000002"},
      { "Ring" :  0, "VesselId": "0", "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000003"},
      { "Ring" :  0, "VesselId": "0", "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000004"},
      { "Ring" :  0, "VesselId": "0", "FEN": 1, "Hybrid" :  2, "HybridId" : "E5533333222222221111111100000005"},
      { "Ring" :  0, "VesselId": "1", "FEN": 2, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000006"},
      { "Ring" :  0, "VesselId": "1", "FEN": 2, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000007"},
      { "Ring" :  0, "VesselId": "1", "FEN": 2, "Hybrid" :  2, "HybridId" : "E5533333222222221111111100000008"},
      { "Ring" :  0, "VesselId": "1", "FEN": 3, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000009"},
      { "Ring" :  0, "VesselId": "1", "FEN": 3, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000010"},
      { "Ring" :  0, "VesselId": "1", "FEN": 3, "Hybrid" :  2, "HybridId" : "E5533333222222221111111100000011"},
      { "Ring" :  0, "VesselId": "2", "FEN": 4, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000012"},
      { "Ring" :  0, "VesselId": "2", "FEN": 4, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000013"},
      { "Ring" :  0, "VesselId": "2", "FEN": 4, "Hybrid" :  2, "HybridId" : "E5533333222222221111111100000014"},
      { "Ring" :  0, "VesselId": "2", "FEN": 5, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000015"},
      { "Ring" :  0, "VesselId": "2", "FEN": 5, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000016"},
      { "Ring" :  0, "VesselId": "2", "FEN": 5, "Hybrid" :  2, "HybridId" : "E5533333222222221111111100000017"},
      { "Ring" :  0, "VesselId": "3", "FEN": 6, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000018"},
      { "Ring" :  0, "VesselId": "3", "FEN": 6, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000019"},
      { "Ring" :  0, "VesselId": "3", "FEN": 6, "Hybrid" :  2, "HybridId" : "E5533333222222221111111100000020"},
      { "Ring" :  0, "VesselId": "3", "FEN": 7, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000021"},
      { "Ring" :  0, "VesselId": "3", "FEN": 7, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000022"},
      { "Ring" :  0, "VesselId": "3", "FEN": 7, "Hybrid" :  2, "HybridId" : "E5533333222222221111111100000023"}
    ],

    "MaxPulseTimeNS" : 71428570,
    "DefaultMinADC": 50,
    "MaxGridsPerEvent": 5,
    "SizeX": 12,
    "SizeY": 51,
    "SizeZ": 16
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


std::vector<uint8_t> SplitEventA {
  // First readout - plane Y - Grids
  0x00, 0x01, 0x14, 0x00,  // Data Header - Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3C,  // GEO 0, TDC 0, VMM 1, CH 60

};

std::vector<uint8_t> SplitEventB {
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

std::vector<uint8_t> BadEventMultipleWires {
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

   // Third readout - plane X & Z - Wires
  0x00, 0x01, 0x14, 0x00,  // Data Header, Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 6 ticks
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3D,  // GEO 0, TDC 0, VMM 0, CH 61
};

std::vector<uint8_t> HighTOFError {
  // First readout - plane Y - Grids
  0x00, 0x01, 0x14, 0x00,  // Data Header - Ring 0, FEN 1
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3C,  // GEO 0, TDC 0, VMM 1, CH 60

  // Second readout - plane Y - Grids
  0x00, 0x01, 0x14, 0x00,  // Data Header - Ring 0, FEN 1
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x02, 0x00, 0x00, 0x00,  // Time LO 2 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3D,  // GEO 0, TDC 0, VMM 1, CH 61

  // Third readout - plane X & Z - Wires
  0x00, 0x01, 0x14, 0x00,  // Data Header, Ring 0, FEN 1
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
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


std::vector<uint8_t> BadEventLargeTimeSpan {
  // First readout - plane Y - Grids
  0x00, 0x01, 0x14, 0x00,  // Data Header - Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3C,  // GEO 0, TDC 0, VMM 1, CH 60

  // Second readout - plane Y - Grids
  0x00, 0x01, 0x14, 0x00,  // Data Header - Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0xFF, 0x04, 0x00, 0x00,  // Time LO 500 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3D,  // GEO 0, TDC 0, VMM 1, CH 61

  // Second readout - plane Y - Grids
  0x00, 0x01, 0x14, 0x00,  // Data Header - Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x14, 0x00, 0x00, 0x00,  // Time LO 20 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3E,  // GEO 0, TDC 0, VMM 1, CH 62

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

  // Second readout - plane X & Z - Wires
  0x00, 0x01, 0x14, 0x00,  // Data Header, Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0x4B, 0x00,  // ADC = 75, over threshold required
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60

  // Third readout - plane X & Z - Wires
  0x00, 0x04, 0x14, 0x00,  // Data Header, Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0x4B, 0x00,  // ADC = 75, under threshold for this specific vessel
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60
};

std::vector<uint8_t> NoEventGridOnly {
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

std::vector<uint8_t> NoEventWireOnly {
  // First readout - plane X & Z - Wires
  0x00, 0x01, 0x14, 0x00,  // Data Header - Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60

  // Second readout - plane X & Z - Wires
  0x00, 0x01, 0x14, 0x00,  // Data Header - Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3D,  // GEO 0, TDC 0, VMM 0, CH 61
};

// clang-format on

class TREXInstrumentTest : public TestBase {
public:
protected:
  struct Counters counters;
  BaseSettings Settings;
  EV44Serializer *serializer;
  TREXInstrument *trex;
  ESSReadout::Parser::PacketHeaderV0 PacketHeader;
  Event TestEvent;           // used for testing generateEvents()
  std::vector<Event> Events; // used for testing generateEvents()

  void SetUp() override {
    Settings.ConfigFile = ConfigFile;
    serializer = new EV44Serializer(115000, "trex");
    counters = {};

    memset(&PacketHeader, 0, sizeof(PacketHeader));

    trex = new TREXInstrument(counters, Settings, serializer);
    trex->setSerializer(serializer);
    trex->ESSReadoutParser.Packet.HeaderPtr = &PacketHeader;
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
TEST_F(TREXInstrumentTest, DumpTofile) {
  Settings.DumpFilePrefix = "deleteme_";
  TREXInstrument TREXDump(counters, Settings, serializer);
  TREXDump.setSerializer(serializer);

  makeHeader(TREXDump.ESSReadoutParser.Packet, GoodEvent);
  auto Res = TREXDump.VMMParser.parse(TREXDump.ESSReadoutParser.Packet);
  TREXDump.processReadouts();

  counters.VMMStats = TREXDump.VMMParser.Stats;
  ASSERT_EQ(Res, 3);
  ASSERT_EQ(counters.VMMStats.Readouts, 3);
}

// Test cases below
TEST_F(TREXInstrumentTest, BadConfig) {
  Settings.ConfigFile = BadConfigFile;
  EXPECT_THROW(TREXInstrument(counters, Settings, serializer),
               std::runtime_error);
}

TEST_F(TREXInstrumentTest, Constructor) {
  ASSERT_EQ(counters.HybridMappingErrors, 0);
}

TEST_F(TREXInstrumentTest, BadRingAndFENError) {
  makeHeader(trex->ESSReadoutParser.Packet, BadRingAndFENError);
  auto Res = trex->VMMParser.parse(trex->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 0);
  counters.VMMStats = trex->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.ErrorRing, 1);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 1);
}

TEST_F(TREXInstrumentTest, GoodEvent) {
  makeHeader(trex->ESSReadoutParser.Packet, GoodEvent);
  auto Res = trex->VMMParser.parse(trex->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 3);
  counters.VMMStats = trex->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.ErrorRing, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);

  // Ring and FEN IDs are within bounds, but Hybrid is not defined in config
  trex->processReadouts();
  ASSERT_EQ(counters.VMMStats.ErrorRing, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);
  ASSERT_EQ(counters.VMMStats.Readouts, 3);

  for (auto &builder : trex->builders) {
    builder.flush(true);
    trex->generateEvents(builder.Events);
  }
  ASSERT_EQ(counters.Events, 1);
}

TEST_F(TREXInstrumentTest, BadMappingError) {
  makeHeader(trex->ESSReadoutParser.Packet, BadMappingError);
  auto Res = trex->VMMParser.parse(trex->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 2);
  counters.VMMStats = trex->VMMParser.Stats;

  ASSERT_EQ(counters.VMMStats.ErrorRing, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);

  trex->processReadouts();
  ASSERT_EQ(counters.VMMStats.ErrorRing, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);
  counters.VMMStats = trex->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.Readouts, 2);

  for (auto &builder : trex->builders) {
    builder.flush(true);
    trex->generateEvents(builder.Events);
  }
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.MappingErrors, 2);
}

TEST_F(TREXInstrumentTest, MaxADC) {
  makeHeader(trex->ESSReadoutParser.Packet, MaxADC);
  auto Res = trex->VMMParser.parse(trex->ESSReadoutParser.Packet);
  counters.VMMStats = trex->VMMParser.Stats;

  // ADC was above VMM threshold of 1023 once
  ASSERT_EQ(counters.VMMStats.ErrorADC, 1);
  ASSERT_EQ(Res, 1);
}

TEST_F(TREXInstrumentTest, MinADC) {
  makeHeader(trex->ESSReadoutParser.Packet, MinADC);
  auto Res = trex->VMMParser.parse(trex->ESSReadoutParser.Packet);
  counters.VMMStats = trex->VMMParser.Stats;
  ASSERT_EQ(Res, 3);
  ASSERT_EQ(counters.VMMStats.ErrorADC, 0);

  trex->processReadouts();
  ASSERT_EQ(counters.MinADC, 2); // ADC was under vessel specific threshold
                                 // once, under general default once
}

TEST_F(TREXInstrumentTest, NoEventGridOnly) {
  makeHeader(trex->ESSReadoutParser.Packet, NoEventGridOnly);
  auto Res = trex->VMMParser.parse(trex->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 2);
  counters.VMMStats = trex->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.ErrorRing, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);

  trex->processReadouts();
  ASSERT_EQ(counters.VMMStats.ErrorRing, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);
  counters.VMMStats = trex->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.Readouts, 2);

  for (auto &builder : trex->builders) {
    builder.flush(true);
    trex->generateEvents(builder.Events);
  }
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.ClustersNoCoincidence, 1);
  ASSERT_EQ(counters.ClustersMatchedGridOnly, 1);
}

TEST_F(TREXInstrumentTest, NoEventWireOnly) {
  makeHeader(trex->ESSReadoutParser.Packet, NoEventWireOnly);
  auto Res = trex->VMMParser.parse(trex->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 2);
  counters.VMMStats = trex->VMMParser.Stats;

  ASSERT_EQ(counters.VMMStats.ErrorRing, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);

  trex->processReadouts();
  ASSERT_EQ(counters.VMMStats.ErrorRing, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);
  ASSERT_EQ(counters.VMMStats.Readouts, 2);

  for (auto &builder : trex->builders) {
    builder.flush(true);
    trex->generateEvents(builder.Events);
  }
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.ClustersNoCoincidence, 1);
  ASSERT_EQ(counters.ClustersMatchedWireOnly, 1);
}

TEST_F(TREXInstrumentTest, NoEvents) {
  Events.push_back(TestEvent);
  trex->generateEvents(Events);
  ASSERT_EQ(counters.Events, 0);
}

TEST_F(TREXInstrumentTest, PixelError) {
  TestEvent.ClusterA.insert({0, 1, 100, 0});
  TestEvent.ClusterB.insert({0, 60000, 100, 1});
  Events.push_back(TestEvent);
  trex->generateEvents(Events);
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.PixelErrors, 1);
}

TEST_F(TREXInstrumentTest, BadEventLargeGridSpan) {
  makeHeader(trex->ESSReadoutParser.Packet, BadEventLargeGridSpan);
  auto Res = trex->VMMParser.parse(trex->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 5);
  counters.VMMStats = trex->VMMParser.Stats;

  ASSERT_EQ(counters.VMMStats.ErrorRing, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);

  trex->processReadouts();
  ASSERT_EQ(counters.VMMStats.ErrorRing, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);
  ASSERT_EQ(counters.VMMStats.Readouts, 5);

  for (auto &builder : trex->builders) {
    builder.flush(true);
    trex->generateEvents(builder.Events);
  }
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.ClustersTooLargeGridSpan, 1);
}

TEST_F(TREXInstrumentTest, NegativeTOF) {
  auto &Packet = trex->ESSReadoutParser.Packet;
  makeHeader(trex->ESSReadoutParser.Packet, GoodEvent);
  Packet.Time.setReference(200, 0);

  auto Res = trex->VMMParser.parse(trex->ESSReadoutParser.Packet);
  counters.VMMStats = trex->VMMParser.Stats;

  trex->processReadouts();
  for (auto &builder : trex->builders) {
    builder.flush(true);
    trex->generateEvents(builder.Events);
  }

  ASSERT_EQ(Res, 3);
  ASSERT_EQ(counters.VMMStats.Readouts, 3);
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.TimeErrors, 1);
}

TEST_F(TREXInstrumentTest, HighTOFError) {
  makeHeader(trex->ESSReadoutParser.Packet, HighTOFError);

  auto Res = trex->VMMParser.parse(trex->ESSReadoutParser.Packet);
  counters.VMMStats = trex->VMMParser.Stats;

  trex->processReadouts();
  for (auto &builder : trex->builders) {
    builder.flush(true);
    trex->generateEvents(builder.Events);
  }

  ASSERT_EQ(Res, 3);
  ASSERT_EQ(counters.VMMStats.Readouts, 3);
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.TOFErrors, 1);
}

TEST_F(TREXInstrumentTest, BadEventLargeTimeSpan) {
  makeHeader(trex->ESSReadoutParser.Packet, BadEventLargeTimeSpan);

  auto Res = trex->VMMParser.parse(trex->ESSReadoutParser.Packet);
  counters.VMMStats = trex->VMMParser.Stats;

  trex->processReadouts();
  for (auto &builder : trex->builders) {
    builder.flush(true);
    trex->generateEvents(builder.Events);
  }

  ASSERT_EQ(Res, 4);
  ASSERT_EQ(counters.VMMStats.Readouts, 4);
  ASSERT_EQ(counters.Events, 1);
  ASSERT_EQ(counters.ClustersNoCoincidence, 1);
}

TEST_F(TREXInstrumentTest, EventCrossPackets) {
  makeHeader(trex->ESSReadoutParser.Packet, SplitEventA);

  auto Res = trex->VMMParser.parse(trex->ESSReadoutParser.Packet);
  counters.VMMStats = trex->VMMParser.Stats;

  trex->processReadouts();
  for (auto &builder : trex->builders) {
    builder.flush();
    trex->generateEvents(builder.Events);
  }
  ASSERT_EQ(Res, 1);

  makeHeader(trex->ESSReadoutParser.Packet, SplitEventB);

  Res = trex->VMMParser.parse(trex->ESSReadoutParser.Packet);
  counters.VMMStats = trex->VMMParser.Stats;

  trex->processReadouts();
  for (auto &builder : trex->builders) {
    builder.flush(true);
    trex->generateEvents(builder.Events);
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
