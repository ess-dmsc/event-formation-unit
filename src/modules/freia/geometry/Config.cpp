// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <freia/geometry/Config.h>

namespace Freia {

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

void Config::loadAndApply() {
  root = from_json_file(FileName);
  apply();
}

void Config::apply() {
  std::string Name;
  try {
    Name = root["Detector"].get<std::string>();
  } catch (...) {
    LOG(INIT, Sev::Error, "Missing 'Detector' field");
    throw std::runtime_error("Missing 'Detector' field");
  }

  if (Name != "Freia") {
    LOG(INIT, Sev::Error, "Instrument configuration is not Freia");
    throw std::runtime_error("Inconsistent Json file - invalid name");
  }

  try {
    MaxPulseTimeNS = root["MaxPulseTimeNS"].get<std::uint32_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxPulseTimeNS");
  }
  LOG(INIT, Sev::Info, "MaxPulseTimeNS {}", MaxPulseTimeNS);

  try {
    MaxGapWire = root["MaxGapWire"].get<std::uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxGapWire");
  }
  LOG(INIT, Sev::Info, "MaxGapWire {}", MaxGapWire);

  try {
    MaxGapStrip = root["MaxGapStrip"].get<std::uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxGapStrip");
  }
  LOG(INIT, Sev::Info, "MaxGapStrip {}", MaxGapStrip);

  try {
    TimeBoxNs = root["TimeBoxNs"].get<std::uint32_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for TimeBoxNs");
  }
  LOG(INIT, Sev::Info, "TimeBoxNs {}", TimeBoxNs);


  try {
    auto PanelConfig = root["Config"];
    unsigned int VMMOffs{0};
    unsigned int FENOffs{0};
    for (auto &Mapping : PanelConfig) {
      auto Ring = Mapping["Ring"].get<unsigned int>();
      auto Offset = Mapping["CassOffset"].get<unsigned int>();
      auto FENs = Mapping["FENs"].get<unsigned int>();

      XTRACE(INIT, DEB, "Ring %d, Offset %d, FENs %d", Ring, Offset, FENs);

      if ((Ring != NumRings) or (Ring > 10)) {
        LOG(INIT, Sev::Error, "Ring configuration error");
        throw std::runtime_error("Inconsistent Json file - ring index mismatch");
      }

      NumFens.push_back(FENs);
      FENOffset.push_back(FENOffs);
      VMMOffset.push_back(VMMOffs);

      VMMOffs += FENs * VMMsPerFEN;
      FENOffs += FENs;
      NumCassettes += FENs * CassettesPerFEN;
      NumRings++;

      LOG(INIT, Sev::Info,
          "JSON config - Detector {}, Ring {}, Offset {}, FENs {}",
          Name, Ring, Offset, FENs);
    }

    NumPixels = NumCassettes * NumWiresPerCassette * NumStripsPerCassette; //
    LOG(INIT, Sev::Info, "JSON config - Detector has {} cassettes and "
    "{} pixels", NumCassettes, NumPixels);


  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Config file: {}",
        FileName);
    throw std::runtime_error("Invalid Json file");
    return;
  }
}

} // namespace Freia
