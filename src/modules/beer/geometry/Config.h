// Copyright (C) 2025 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief BEER instrument config - extends CBM config for BEER detector
///
//===----------------------------------------------------------------------===//

#pragma once

#include <modules/cbm/CbmTypes.h>
#include <modules/cbm/SchemaType.h>
#include <modules/cbm/geometry/Config.h>

namespace beer {

///
/// \class Config
/// \brief BEER-specific configuration extending CBM Config
///
/// This class reuses the CBM configuration infrastructure but:
/// - Uses "BEER" as the instrument name
/// - Presets Type=EVENT_2D and Schema=EV44 for all topology entries
/// - Only supports EVENT_2D topology type
///
class Config : public cbm::Config {
public:
  /// \brief Default constructor with preset instrument type
  Config() : cbm::Config(DetectorType::BEER) {}

  /// \brief Constructor that loads configuration from a JSON file
  Config(const std::string &ConfigFile)
      : cbm::Config(ConfigFile, DetectorType::BEER){};

protected:
  /// \brief Override to preset Type=EVENT_2D and Schema=EV44
  /// Only requires FEN, Channel, and Source in JSON
  cbm::TopologyEntryData
  parseTopologyEntryData(const nlohmann::json &Module) override {

    cbm::TopologyEntryData data;

    try {
      data.FEN = Module["FEN"].get<int>();
      data.Channel = Module["Channel"].get<int>();
      data.Source = Module["Source"].get<std::string>();

      // BEER presets: Type and Schema are fixed
      data.Type = cbm::CbmType(cbm::CbmType::EVENT_2D).toString();
      data.Schema = cbm::SchemaType(cbm::SchemaType::EV44).toString();
    } catch (...) {
      errorExit("Malformed 'Topology' section (Need FEN, Channel, and Source)");
    }

    return data;
  }
};

} // namespace beer
