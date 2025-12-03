// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Monitor config
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/config/Config.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/JsonFile.h>
#include <common/memory/HashMap2D.h>
#include <common/readout/ess/Parser.h>
#include <modules/cbm/CbmTypes.h>

#include <memory>
#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace cbm {

// enumeration of how to aggregate pulses on CBM
// SUM will be used as default where it will sum
// up one pulse.
enum AggregationType {
  SUM = 0x00,     // < Calculate sum of X pulses 
  AVG = 0x01      // < Calculate average of X pulses
};

struct Topology {
  const int FEN{0};
  const int Channel{0};
  const std::string Source{"CBM"};
  const CbmType Type{CbmType::EVENT_0D};
  union {
    int param1{0};
    int pixelOffset;
    int maxTofBin;
    int width;
  };
  union {
    int param2{0};
    int BinCount;
    int height;
  };
  union {
    int param3{0};
    int AggregatedFrames;
  };
  union {
    int param4{0};
    int AggregationMode;
  };

  Topology() = default;

  // clang-format off
  Topology(int FEN, 
           int Channel, 
           const std::string &Source,
           const CbmType &Type, 
           int param1)
    : FEN(FEN)
    , Channel(Channel)
    , Source(Source)
    , Type(Type)
    , param1(param1){};

  Topology(int FEN, 
           int Channel, 
           const std::string &Source,
           const CbmType &Type, 
           int param1, 
           int param2,
           int param3, 
           int param4)
    : FEN(FEN), Channel(Channel)
    , Source(Source)
    , Type(Type)
    , param1(param1)
    , param2(param2)
    , param3(param3)
    , param4(param4){};
  // clang-format on
};

class Config : public Configurations::Config {

  void errorExit(const std::string &ErrMsg);

public:
  Config(){};

  // Load and apply the json config
  Config(const std::string &ConfigFile)
    : Configurations::Config(ConfigFile) {};

  // load file into json object and apply
  void loadAndApply();

  // Apply the loaded json file
  void apply();

  // clang-format off
  //
  // Parameters (eventually) obtained from JSON config file
  struct {
    uint8_t TypeSubType{DetectorType::CBM};
    uint32_t MaxTOFNS{20 * 71'428'571};             // < Twenty 14Hz pulses
    uint32_t MaxPulseTimeDiffNS{5 * 71'428'571};    // < Five 14Hz pulses
    uint8_t MonitorRing{11};                        // < Ring number for the monitors
    uint8_t NumberOfMonitors{1};                    // < Number of monitors in the config
    uint8_t MaxFENId{10};                           // < Maximum number of FEN IDs
    uint8_t NumOfFENs{11};                          // < Number of FENs (MaxId + 1)
    uint8_t AggregationMode{AggregationType::SUM};  // < How to aggregate pulses. Default is SUM
    uint8_t AggregatedFrames{1};                    // < Number of pulse to sum in a histogram
                                                    //   When set to one and with the default SUM mode,
                                                    //   this will be one frame one histogram.
  } CbmParms;
  // clang-format on

  std::unique_ptr<HashMap2D<Topology>> TopologyMapPtr;
};

} // namespace cbm
