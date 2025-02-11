// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/debug/Error.h>
#include <fmt/core.h>
#include <stdexcept>
#include <tbl3he/geometry/Tbl3HeConfig.h>
#include <unistd.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

namespace Caen {

///
Tbl3HeConfig::Tbl3HeConfig() {}

Tbl3HeConfig::Tbl3HeConfig(const std::string &ConfigFile)
    : ConfigFile(ConfigFile), ConfigFileName(ConfigFile) {
  XTRACE(INIT, DEB, "Loading json file");
  root = from_json_file(ConfigFile);
}

void Tbl3HeConfig::errorExit(const std::string &ErrMsg) {
  LOG(INIT, Sev::Error, ErrMsg);
  XTRACE(INIT, ERR, "%s\n", ErrMsg.c_str());
  sleep(1);
  throw std::runtime_error(ErrMsg);
}

void Tbl3HeConfig::parseConfig() {

  json_check_keys("Mandatory keys", root,
                  {"Detector", "Resolution", "MaxPulseTimeNS", "MaxTOFNS",
                   "NumOfFENs", "MinValidAmplitude", "MaxGroup", "Topology"});

  Parms.InstrumentName = root["Detector"].get<std::string>();
  if (Parms.InstrumentName != "tbl3he") {
    errorExit(fmt::format("Invalid instrument name ({}) for tbl3he",
                          Parms.InstrumentName));
  }

  try {
    // Assumed the same for all tubes
    Parms.Resolution = root["Resolution"].get<int>();
    XTRACE(INIT, DEB, "Resolution %d", Parms.Resolution);

    Parms.MaxPulseTimeNS = root["MaxPulseTimeNS"].get<unsigned int>();
    LOG(INIT, Sev::Info, "MaxPulseTimeNS: {}", Parms.MaxPulseTimeNS);

    Parms.MaxTOFNS = root["MaxTOFNS"].get<unsigned int>();
    LOG(INIT, Sev::Info, "MaxTOFNS: {}", Parms.MaxTOFNS);

    Parms.NumOfFENs = root["NumOfFENs"].get<unsigned int>();
    LOG(INIT, Sev::Info, "NumOfFENs: {}", Parms.NumOfFENs);

    Parms.MinValidAmplitude = root["MinValidAmplitude"].get<unsigned int>();
    LOG(INIT, Sev::Info, "MinValidAmplitude: {}", Parms.MinValidAmplitude);

    Parms.MaxGroup = root["MaxGroup"].get<unsigned int>();
    LOG(INIT, Sev::Info, "MaxGroup: {}", Parms.MaxGroup);

    // Run through the Topology section
    auto Configs = root["Topology"];

    if (Parms.NumOfFENs != (int)Configs.size()) {
      errorExit(fmt::format("RING/FEN topology mismatch, NumOfFEN: {} != Config size: {}",
                            Parms.NumOfFENs, Configs.size()));
    }

    TopologyMapPtr.reset(new HashMap2D<Topology>(Parms.NumOfFENs));

    for (auto &elt : Configs) {
      json_check_keys("Mandatory Topology keys", elt, {"Ring", "FEN", "Bank"});

      int Ring = elt["Ring"].get<int>();
      int FEN = elt["FEN"].get<int>();
      int Bank = elt["Bank"].get<int>();

      if ((Ring < Parms.MinRing) or (Ring > Parms.MaxRing)) {
        errorExit(fmt::format("Invalid ring: %d", Ring));
      }

      if (TopologyMapPtr->isValue(Ring, FEN)) {
        errorExit(fmt::format("Duplicate entry for Ring {} FEN {}", Ring, FEN));
      }

      auto topo = std::make_unique<Topology>(Bank);
      TopologyMapPtr->add(Ring, FEN, topo);
    }

  } catch (fmt::format_error &e) {
    LOG(INIT, Sev::Critical,
        "JSON config - code error during fm::format() call: {}", e.what());
    throw std::runtime_error(std::string("(fmt::fomat_error) ") + e.what());
  } catch (std::exception &e) {
    RETHROW_WITH_HINT(e);
  }
}

} // namespace Caen
