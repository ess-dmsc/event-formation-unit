// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include "CbmTypes.h"
#include <modules/cbm/geometry/Config.h>

namespace cbm {

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

void Config::errorExit(const std::string &ErrMsg) {
  LOG(INIT, Sev::Error, ErrMsg);
  throw std::runtime_error(ErrMsg);
}

void Config::loadAndApply() {
  loadFromFile();
  apply();
}

void Config::apply() {
  std::string DetectorName;

  setMask(LOG | CHECK);
  assign("Detector", DetectorName);

  if (DetectorName != "CBM") {
    LOG(INIT, Sev::Error, "Detector name mismatch, expected CBM");
    throw std::runtime_error("Detector name mismatch, expected CBM");
  }

  setMask(LOG);
  assign("TypeSubType", Parms.TypeSubType);
  assign("MaxPulseTimeDiffNS", Parms.MaxPulseTimeDiffNS);
  assign("MaxTOFNS", Parms.MaxTOFNS);
  assign("MonitorRing", Parms.MonitorRing);

  setMask(LOG | CHECK);
  assign("MaxFENId", Parms.MaxFENId);

  // Number of FENs must must be 1 even is MaxFENId is 0
  Parms.NumOfFENs = Parms.MaxFENId + 1;

  TopologyMapPtr.reset(new HashMap2D<Topology>(Parms.NumOfFENs));
  if (!root().contains("Topology")) {
    throw std::runtime_error("No 'Topology' section found in the "
                             "configuration. Cannot setup Beam Monitors");
  }
  const nlohmann::json Modules = root()["Topology"];

  // Temporary map storage to check for duplicates of the two unique keys
  int Entry{0};
  for (const auto &Module : Modules) {
    // Initialize the parameters with invalid values which are
    // not used in the MAP to recall topology object
    int FEN{-1};
    int Channel{-1};
    std::string Source{""};
    std::string Type{""};

    try {
      FEN = Module["FEN"].get<int>();
      Channel = Module["Channel"].get<int>();
      Type = Module["Type"].get<std::string>();
      Source = Module["Source"].get<std::string>();

    } catch (...) {
      std::runtime_error(
          "Malformed 'Topology' section (Need FEN, Channel, Type and"
          "Source)");
    }

    // Check for array sizes and duplicate entries
    if (FEN > Parms.MaxFENId) {
      errorExit(fmt::format("Entry: {}, Invalid FEN: {} Max: {}", Entry, FEN,
                            Parms.MaxFENId));
    }

    if (TopologyMapPtr->isValue(FEN, Channel)) {
      errorExit(fmt::format("Entry: {}, Duplicate entry for FEN {} Channel {}",
                            Entry, FEN, Channel));
    }

    // Add parameter to the temporary map
    CbmType MonitorType = CbmType::EVENT_0D;
    try {
      MonitorType = CbmType(Type);
    } catch (...) {
      errorExit(fmt::format("Entry: {}, Invalid Type: {} is not a CBM Type",
                            Entry, Type));
    }

    int param1{0};
    int param2{0};

    if (MonitorType == CbmType::EVENT_0D) {

      try {
        param1 = Module["PixelOffset"].get<int>();
      } catch (...) {
        errorExit(fmt::format(
            "Entry: {}, Malformed 'Topology' section for {} Type (Need "
            "PixelOffset, PixelRange)",
            Entry, MonitorType.toString()));
      }
    }

    if (MonitorType == CbmType::IBM) {

      try {
        param1 = Module["MaxTofBin"].get<int>();
        param2 = Module["BinCount"].get<int>();
      } catch (...) {
        errorExit(fmt::format(
            "Entry: {}, Malformed 'Topology' section for {} Type (Need "
            "MaxTofBin, BinCount)",
            Entry, MonitorType.toString()));
      }
    }

    auto topo = std::make_unique<Topology>(FEN, Channel, Source, MonitorType,
                                           param1, param2);
    TopologyMapPtr->add(FEN, Channel, topo);

    XTRACE(INIT, ALW, "Entry %02d, FEN %02d, Channel %02d, Source %s Type %s",
           Entry, FEN, Channel, Source.c_str(), Type.c_str());

    // Count the number of valid entries
    Entry++;
  }

  Parms.NumberOfMonitors = Entry;
}

} // namespace cbm
