// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/Statistics.h>
#include <common/kafka/EV44Serializer.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/HeaderFactory.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <memory>
#include <nmx/NMXInstrument.h>
#include <stdio.h>
#include <string.h>

using namespace Nmx;
using namespace ESSReadout;
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

std::string BadConfig2File{"deleteme_nmx_instr_config_bad2.json"};
std::string BadConfig2Str = R"(
  {
  "Detector" : "NMX",
  "InstrumentGeometry" : "Invalid",
}
)";

std::string ConfigFile{"deleteme_nmx_instr_config.json"};
std::string ConfigStr = R"(
{
  "Detector" : "NMX",
  "InstrumentGeometry" : "NMX",
  "MaxSpanX" : 3,
  "MinSpanX" : 2,
  "MaxSpanY" : 3,
  "MinSpanY" : 1,
  "MaxGapX" : 2,
  "MaxGapY" : 2,
  "DefaultMinADC":50,
  "Config" : [
    {
      "Ring" : 0, "FEN": 0, "Hybrid" : 0, "Plane" : 0, "Offset" : 0,
      "ReversedChannels" : false, "Panel" : 0,
      "HybridId" : "E5533333222222221111111100000000"
    },
    {
      "Ring" : 0, "FEN": 0, "Hybrid" : 1, "Plane" : 1, "Offset" : 0,
      "ReversedChannels" : false, "Panel" : 0,
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
  // First readout - plane X
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60

  // Second readout - plane X
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x02, 0x00, 0x00, 0x00,  // Time LO 2 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3D,  // GEO 0, TDC 0, VMM 0, CH 61

  // Third readout - plane Y
  0x00, 0x00, 0x14, 0x00,  // Data Header, Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3C,  // GEO 0, TDC 0, VMM 1, CH 60

};

std::vector<uint8_t> BadEventSmallXSpan {
  // First readout - plane X
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
  0x00, 0x00, 0x02, 0x3D,  // GEO 0, TDC 0, VMM 1, CH 61

  // Third readout - plane Y
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

   // Fourth readout - plane X
  0x00, 0x00, 0x14, 0x00,  // Data Header, Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3D,  // GEO 0, TDC 0, VMM 0, CH 61

};


std::vector<uint8_t> HighTOFError {
  // First readout - plane Y
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3C,  // GEO 0, TDC 0, VMM 1, CH 60

  // Second readout - plane X
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x02, 0x00, 0x00, 0x00,  // Time LO 2 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3D,  // GEO 0, TDC 0, VMM 0, CH 61

  // Third readout - plane X
  0x00, 0x00, 0x14, 0x00,  // Data Header, Ring 0, FEN 0
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60
};


std::vector<uint8_t> BadEventLargeXSpan {
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

  // Second readout - plane X
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x02, 0x00, 0x00, 0x00,  // Time LO 2 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x0F,  // GEO 0, TDC 0, VMM 1, CH 15

  // Second readout - plane X
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x02, 0x00, 0x00, 0x00,  // Time LO 2 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3F,  // GEO 0, TDC 0, VMM 1, CH 63

  // Third readout - plane X
  0x00, 0x00, 0x14, 0x00,  // Data Header, Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
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
  0x00, 0x00, 0x02, 0x1E,  // GEO 0, TDC 0, VMM 1, CH 30

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

  // Second readout - plane X
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

  // Third readout - plane X
  0x00, 0x00, 0x14, 0x00,  // Data Header - Ring 0, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x14, 0x00, 0x00, 0x00,  // Time LO 20 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3E,  // GEO 0, TDC 0, VMM 0, CH 62

  // Fourth readout - plane X
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
  BaseSettings Settings;
  std::unique_ptr<Statistics> Stats;
  std::unique_ptr<ESSReadout::Parser> ESSHeaderParser;
  EV44Serializer serializer{115000, "nmx"};
  std::unique_ptr<NMXInstrument> nmx;
  std::unique_ptr<TestHeaderFactory> headerFactory;
  Event TestEvent;           // used for testing generateEvents()
  std::vector<Event> Events; // used for testing generateEvents()

  void SetUp() override {
    Settings.ConfigFile = ConfigFile;
    counters = {};

    headerFactory = std::make_unique<TestHeaderFactory>();
    Stats = std::make_unique<Statistics>();
    ESSHeaderParser = std::make_unique<ESSReadout::Parser>(*Stats);
    nmx = std::make_unique<NMXInstrument>(counters, Settings, serializer,
                                          *ESSHeaderParser);

    ESSHeaderParser->Packet.HeaderPtr = headerFactory->createHeader(Parser::V0);
  }
  void TearDown() override {}

  void makeHeader(Parser::PacketDataV0 &Packet,
                  std::vector<uint8_t> &testdata) {
    Packet.HeaderPtr = headerFactory->createHeader(Parser::V0);
    Packet.DataPtr = (char *)&testdata[0];
    Packet.DataLength = testdata.size();
    Packet.Time.setReference(ESSTime(0, 0));
    Packet.Time.setPrevReference(ESSTime(0, 0));
  }
};

// Test cases below
TEST_F(NMXInstrumentTest, BadConfig) {
  Settings.ConfigFile = BadConfigFile;
  EXPECT_THROW(NMXInstrument(counters, Settings, serializer, *ESSHeaderParser),
               std::runtime_error);
}

TEST_F(NMXInstrumentTest, BadConfig2) {
  Settings.ConfigFile = BadConfig2File;
  EXPECT_THROW(NMXInstrument(counters, Settings, serializer, *ESSHeaderParser),
               nlohmann::detail::parse_error);
}

TEST_F(NMXInstrumentTest, Constructor) {
  ASSERT_EQ(counters.HybridMappingErrors, 0);
}

TEST_F(NMXInstrumentTest, BadRingAndFENError) {
  makeHeader(ESSHeaderParser->Packet, BadRingAndFENError);
  auto Res = nmx->VMMParser.parse(ESSHeaderParser->Packet);
  ASSERT_EQ(Res, 0);
  counters.VMMStats = nmx->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.ErrorFiber, 1);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 1);
}

TEST_F(NMXInstrumentTest, GoodEvent) {
  makeHeader(ESSHeaderParser->Packet, GoodEvent);
  auto Res = nmx->VMMParser.parse(ESSHeaderParser->Packet);
  ASSERT_EQ(Res, 3);
  counters.VMMStats = nmx->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.ErrorFiber, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);

  // Ring and FEN IDs are within bounds, but Hybrid is not defined in config
  nmx->processReadouts();
  ASSERT_EQ(counters.VMMStats.ErrorFiber, 0);
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
  makeHeader(ESSHeaderParser->Packet, MaxADC);
  auto Res = nmx->VMMParser.parse(ESSHeaderParser->Packet);
  counters.VMMStats = nmx->VMMParser.Stats;

  // ADC was above VMM threshold of 1023 once
  ASSERT_EQ(counters.VMMStats.ErrorADC, 1);
  ASSERT_EQ(Res, 1);
}

TEST_F(NMXInstrumentTest, MinADC) {
  makeHeader(ESSHeaderParser->Packet, MinADC);
  auto Res = nmx->VMMParser.parse(ESSHeaderParser->Packet);
  counters.VMMStats = nmx->VMMParser.Stats;
  ASSERT_EQ(Res, 3);
  ASSERT_EQ(counters.VMMStats.ErrorADC, 0);

  nmx->processReadouts();
  ASSERT_EQ(counters.MinADC, 2); // ADC was under vessel specific threshold
                                 // once, under general default once
}

TEST_F(NMXInstrumentTest, NoEventYOnly) {
  makeHeader(ESSHeaderParser->Packet, NoEventYOnly);
  auto Res = nmx->VMMParser.parse(ESSHeaderParser->Packet);
  ASSERT_EQ(Res, 2);
  counters.VMMStats = nmx->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.ErrorFiber, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);

  nmx->processReadouts();
  ASSERT_EQ(counters.VMMStats.ErrorFiber, 0);
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
  makeHeader(ESSHeaderParser->Packet, NoEventXOnly);
  auto Res = nmx->VMMParser.parse(ESSHeaderParser->Packet);
  ASSERT_EQ(Res, 2);
  counters.VMMStats = nmx->VMMParser.Stats;

  ASSERT_EQ(counters.VMMStats.ErrorFiber, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);

  nmx->processReadouts();
  ASSERT_EQ(counters.VMMStats.ErrorFiber, 0);
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
  TestEvent.ClusterA.insert({0, 2, 100, 0});
  TestEvent.ClusterB.insert({0, 60000, 100, 1});
  Events.push_back(TestEvent);
  nmx->generateEvents(Events);
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.PixelErrors, 1);
}

TEST_F(NMXInstrumentTest, BadEventLargeYSpan) {
  makeHeader(ESSHeaderParser->Packet, BadEventLargeYSpan);
  auto Res = nmx->VMMParser.parse(ESSHeaderParser->Packet);
  ASSERT_EQ(Res, 5);
  counters.VMMStats = nmx->VMMParser.Stats;

  ASSERT_EQ(counters.VMMStats.ErrorFiber, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);

  nmx->processReadouts();
  ASSERT_EQ(counters.VMMStats.ErrorFiber, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);
  ASSERT_EQ(counters.VMMStats.Readouts, 5);

  for (auto &builder : nmx->builders) {
    builder.flush(true);
    nmx->generateEvents(builder.Events);
  }
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.ClustersTooLargeSpanY, 1);
}

TEST_F(NMXInstrumentTest, BadEventSmallXSpan) {
  makeHeader(ESSHeaderParser->Packet, BadEventSmallXSpan);
  auto Res = nmx->VMMParser.parse(ESSHeaderParser->Packet);
  ASSERT_EQ(Res, 3);
  counters.VMMStats = nmx->VMMParser.Stats;

  ASSERT_EQ(counters.VMMStats.ErrorFiber, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);

  nmx->processReadouts();
  ASSERT_EQ(counters.VMMStats.ErrorFiber, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);
  ASSERT_EQ(counters.VMMStats.Readouts, 3);

  for (auto &builder : nmx->builders) {
    builder.flush(true);
    nmx->generateEvents(builder.Events);
  }
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.ClustersTooSmallSpanX, 1);
}

TEST_F(NMXInstrumentTest, BadEventLargeXSpan) {
  makeHeader(ESSHeaderParser->Packet, BadEventLargeXSpan);
  auto Res = nmx->VMMParser.parse(ESSHeaderParser->Packet);
  ASSERT_EQ(Res, 5);
  counters.VMMStats = nmx->VMMParser.Stats;

  ASSERT_EQ(counters.VMMStats.ErrorFiber, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);

  nmx->processReadouts();
  ASSERT_EQ(counters.VMMStats.ErrorFiber, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);
  ASSERT_EQ(counters.VMMStats.Readouts, 5);

  for (auto &builder : nmx->builders) {
    builder.flush(true);
    nmx->generateEvents(builder.Events);
  }
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.ClustersTooLargeSpanX, 1);
}

TEST_F(NMXInstrumentTest, NegativeTOF) {
  auto &Packet = ESSHeaderParser->Packet;
  makeHeader(ESSHeaderParser->Packet, GoodEvent);
  Packet.Time.setReference(ESSTime(200, 0));

  auto Res = nmx->VMMParser.parse(ESSHeaderParser->Packet);
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
  makeHeader(ESSHeaderParser->Packet, HighTOFError);

  auto Res = nmx->VMMParser.parse(ESSHeaderParser->Packet);
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
  makeHeader(ESSHeaderParser->Packet, BadEventLargeTimeSpan);

  auto Res = nmx->VMMParser.parse(ESSHeaderParser->Packet);
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
  makeHeader(ESSHeaderParser->Packet, SplitEventA);

  auto Res = nmx->VMMParser.parse(ESSHeaderParser->Packet);
  counters.VMMStats = nmx->VMMParser.Stats;

  nmx->processReadouts();
  for (auto &builder : nmx->builders) {
    builder.flush();
    nmx->generateEvents(builder.Events);
  }
  ASSERT_EQ(Res, 1);

  makeHeader(ESSHeaderParser->Packet, SplitEventB);

  Res = nmx->VMMParser.parse(ESSHeaderParser->Packet);
  counters.VMMStats = nmx->VMMParser.Stats;

  nmx->processReadouts();
  for (auto &builder : nmx->builders) {
    builder.flush(true);
    nmx->generateEvents(builder.Events);
  }

  ASSERT_EQ(Res, 3);
  ASSERT_EQ(counters.VMMStats.Readouts, 4);
  ASSERT_EQ(counters.Events, 1);
}

int main(int argc, char **argv) {
  saveBuffer(ConfigFile, (void *)ConfigStr.c_str(), ConfigStr.size());
  saveBuffer(BadConfigFile, (void *)BadConfigStr.c_str(), BadConfigStr.size());
  saveBuffer(BadConfig2File, (void *)BadConfig2Str.c_str(),
             BadConfig2Str.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(ConfigFile);
  deleteFile(BadConfigFile);
  deleteFile(BadConfig2File);
  return RetVal;
}
