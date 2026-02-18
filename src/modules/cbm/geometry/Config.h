// Copyright (C) 2022 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Monitor config
///
//===----------------------------------------------------------------------===//

#pragma once

#include "common/types/DetectorType.h"
#include <common/JsonFile.h>
#include <common/config/Config.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/memory/HashMap2D.h>
#include <common/readout/ess/Parser.h>
#include <modules/cbm/CbmTypes.h>
#include <modules/cbm/SchemaType.h>

#include <memory>
#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace cbm {

/// \enum AggregationType
/// \brief enumeration of how to aggregate pulses on CBM
/// SUM will be used as default where it will sum
/// up one pulse.
enum AggregationType {
  SUM = 0x00, // < Calculate sum of X pulses
  AVG = 0x01  // < Calculate average of X pulses
};

struct Topology {
  const int FEN{0};
  const int Channel{0};
  const std::string Source{"CBM"};
  const CbmType Type{CbmType::EVENT_0D};
  const SchemaType Schema{SchemaType::EV44};

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
           const SchemaType &Schema,
           int param1)
    : FEN(FEN)
    , Channel(Channel)
    , Source(Source)
    , Type(Type)
    , Schema(Schema)
    , param1(param1){};

  Topology(int FEN, 
           int Channel, 
           const std::string &Source,
           const CbmType &Type, 
           const SchemaType &Schema,
           int param1, 
           int param2,
           int param3, 
           int param4)
    : FEN(FEN), Channel(Channel)
    , Source(Source)
    , Type(Type)
    , Schema(Schema)
    , param1(param1)
    , param2(param2)
    , param3(param3)
    , param4(param4){};
  // clang-format on
};

/// \brief Basic topology entry data parsed from JSON
struct TopologyEntryData {
  int FEN{-1};
  int Channel{-1};
  std::string Source;
  std::string Type;
  std::string Schema;
};

class Config : public Configurations::Config {

public:
  Config(const DetectorType &Instrument = DetectorType::CBM)
      : Instrument(Instrument){};

  // Load and apply the json config
  Config(const std::string &ConfigFile,
         const DetectorType &Instrument = DetectorType::CBM)
      : Configurations::Config(ConfigFile), Instrument(Instrument){};

  virtual ~Config() = default;

  // load file into json object and apply
  void loadAndApply();

  // Apply the loaded json file
  void apply();

  // clang-format off
  //
  // Parameters (eventually) obtained from JSON config file
  struct {
    uint32_t MaxTOFNS{20 * 71'428'571};             // < Twenty 14Hz pulses
    uint32_t MaxPulseTimeDiffNS{5 * 71'428'571};    // < Five 14Hz pulses
    uint8_t MonitorRing{11};                        // < Ring number for the monitors
    uint8_t MaxRing{11};                            // < Maximum valid Ring value (optional, defaults to 11)
    uint8_t NumberOfMonitors{1};                    // < Number of monitors in the config
    uint8_t MaxFENId{10};                           // < Maximum number of FEN IDs
    uint8_t NumOfFENs{11};                          // < Number of FENs (MaxId + 1)
    uint8_t AggregationMode{AggregationType::SUM};  // < How to aggregate pulses. Default is SUM
    uint8_t AggregatedFrames{1};                    // < Number of pulse to sum in a histogram
                                                    //   When set to one and with the default SUM mode,
                                                    //   this will be one frame one histogram.
    bool NormalizeIBMReadouts{true};                // < Normalize IBM ADC value readout. 
                                                    //   Default is enabled.
  } CbmParms;
  // clang-format on

  const DetectorType Instrument;

  std::unique_ptr<HashMap2D<Topology>> TopologyMapPtr;

protected:
  /// \brief Exit with error message
  void errorExit(const std::string &ErrMsg);

  /// \brief Parse basic topology entry fields from JSON
  /// \param Module The JSON object for one topology entry
  /// \return TopologyEntryData with parsed fields
  ///
  virtual TopologyEntryData
  parseTopologyEntryData(const nlohmann::json &Module);
};

} // namespace cbm
