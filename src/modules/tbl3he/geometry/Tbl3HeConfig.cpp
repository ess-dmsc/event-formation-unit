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

  std::string InstrumentName;
  Json::checkKeys("Mandatory keys", root(),
                  {"Detector", "Resolution", 
                   "NumOfFENs", "MinValidAmplitude", "Topology"});

  // Get detector/instrument name
  setMask(CHECK | XTRACE);
  assign("Detector", InstrumentName);
  if (InstrumentName != "tbl3he") {
    errorExit(
        fmt::format("Invalid instrument name ({}) for tbl3he", InstrumentName));
  }

  try {
    setMask(LOG);
    assign("NumOfFENs", Params.NumOfFENs);
    assign("MinValidAmplitude", Params.MinValidAmplitude);

    // Run through the Topology section
    auto Configs = root()["Topology"];

    if (Params.NumOfFENs != (int)Configs.size()) {
      errorExit(fmt::format(
          "RING/FEN topology mismatch, NumOfFEN: {} != Config size: {}",
          Params.NumOfFENs, Configs.size()));
    }

    TopologyMapPtr.reset(new HashMap2D<Topology>(Params.NumOfFENs));

    for (auto &elt : Configs) {
      Json::checkKeys("Mandatory Topology keys", elt, {"Ring", "FEN", "Bank"});

      int Ring = elt["Ring"].get<int>();
      int FEN = elt["FEN"].get<int>();
      int Bank = elt["Bank"].get<int>();

      if ((Ring < Params.MinRing) or (Ring > Params.MaxRing)) {
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
