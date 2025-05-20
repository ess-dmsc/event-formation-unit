// Copyright (C) 2024 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief use nlohmann::json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <common/debug/Error.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <tbl3he/geometry/Tbl3HeConfig.h>

#include <fmt/core.h>

#include <stdexcept>
#include <unistd.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

Tbl3HeConfig::Tbl3HeConfig() {}

Tbl3HeConfig::Tbl3HeConfig(const std::string &ConfigFile)
    : Configurations::Config(ConfigFile) {
  XTRACE(INIT, DEB, "Loading json file");
  loadFromFile();
  XTRACE(INIT, DEB, "json file loaded ");
}

void Tbl3HeConfig::errorExit(const std::string &ErrMsg) {
  LOG(INIT, Sev::Error, ErrMsg);
  XTRACE(INIT, ERR, "%s\n", ErrMsg.c_str());
  sleep(1);
  throw std::runtime_error(ErrMsg);
}

void Tbl3HeConfig::parseConfig() {
  json_check_keys("Mandatory keys", root(),
                  {"Detector", "Resolution", "MaxPulseTimeNS", "MaxTOFNS",
                   "NumOfFENs", "MinValidAmplitude", "MaxGroup", "Topology"});

  // Get detector/instrument name
  setMask(CHECK | XTRACE);
  assign("Detector", Parms.InstrumentName);
  if (Parms.InstrumentName != "tbl3he") {
    errorExit(fmt::format("Invalid instrument name ({}) for tbl3he",
                          Parms.InstrumentName));
  }

  try {
    // Assumed the same for all tubes
    assign("Resolution",        Parms.Resolution);

    setMask(LOG);
    assign("MaxPulseTimeNS",    Parms.MaxPulseTimeNS);
    assign("MaxTOFNS",          Parms.MaxTOFNS);
    assign("NumOfFENs",         Parms.NumOfFENs);
    assign("MinValidAmplitude", Parms.MinValidAmplitude);
    assign("MaxGroup",          Parms.MaxGroup);

    // Run through the Topology section
    auto Configs = root()["Topology"];

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

      auto topology = std::make_unique<Topology>(Bank);
      TopologyMapPtr->add(Ring, FEN, topology);
    }

  } catch (fmt::format_error &e) {
    LOG(INIT, Sev::Critical,
        "JSON config - code error during fm::format() call: {}", e.what());
    throw std::runtime_error(std::string("(fmt::format_error) ") + e.what());
  } catch (std::exception &e) {
    RETHROW_WITH_HINT(e);
  }
}

} // namespace Caen
