// Copyright (C) 2022-2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <modules/cbm/geometry/Config.h>

namespace cbm {

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

void Config::errorExit(const std::string &ErrMsg) {
  LOG(INIT, Sev::Error, ErrMsg);
  throw std::runtime_error(ErrMsg);
}

void Config::loadAndApply() {
  root = from_json_file(FileName);
  apply();
}

void Config::apply() {
  std::string DetectorName;
  try {
    DetectorName = root["Detector"].get<std::string>();
  } catch (...) {
    LOG(INIT, Sev::Error, "Missing 'Detector' field");
    throw std::runtime_error("Missing 'Detector' field");
  }

  if (DetectorName != "CBM") {
    LOG(INIT, Sev::Error, "Detector name mismatch, expected CBM");
    throw std::runtime_error("Detector name mismatch, expected CBM");
  }

  try {
    Parms.TypeSubType = root["TypeSubType"].get<std::uint8_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for TypeSubType");
  }
  LOG(INIT, Sev::Info, "TypeSubType {}", Parms.TypeSubType);

  try {
    Parms.MaxPulseTimeDiffNS = root["MaxPulseTimeDiffNS"].get<std::uint32_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxPulseTimeDiffNS");
  }
  LOG(INIT, Sev::Info, "MaxPulseTimeDiffNS {}", Parms.MaxPulseTimeDiffNS);

  try {
    Parms.MaxTOFNS = root["MaxTOFNS"].get<std::uint32_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxTOFNS");
  }
  LOG(INIT, Sev::Info, "MaxTOFNS {}", Parms.MaxTOFNS);

  try {
    Parms.MonitorRing = root["MonitorRing"].get<std::uint8_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MonitorRing");
  }
  LOG(INIT, Sev::Info, "MonitorRing {}", Parms.MonitorRing);

  try {
    Parms.MaxFENId = root["MaxFENId"].get<int>();

    // Number of FENs must must be 1 even is MaxFENId is 0
    Parms.NumOfFENs = Parms.MaxFENId + 1;
  } catch (...) {
    LOG(INIT, Sev::Error, "MaxFENId not specified");
    throw std::runtime_error("MaxFENId not specified");
  }
  LOG(INIT, Sev::Info, "MaxFENId {}", Parms.MaxFENId);

  TopologyMapPtr.reset(new HashMap2D<Topology>(Parms.NumOfFENs));

  auto TopologyIt = root.find("Topology");
  if (TopologyIt == root.end()) {
    throw std::runtime_error("No 'Topology' section found in the "
                             "configuration. Cannot setup Beam Monitors");
  }

  int Entry{0};
  nlohmann::json Modules;
  Modules = root["Topology"];

  // temporary map storage to check for duplicates ofthe two unique keys

  for (auto &Module : Modules) {
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
    CbmType MonitorType = CbmType::TTL;
    try {
      MonitorType = CbmType(Type);
    } catch (...) {
      errorExit(fmt::format("Entry: {}, Invalid Type: {} is not a CBM Type",
                            Entry, Type));
    }

    int param1{0};
    int param2{0};

    if (MonitorType == CbmType::TTL) {

      try {
        param1 = Module["PixelOffset"].get<int>();
      } catch (...) {
        errorExit(fmt::format(
            "Entry: {}, Malformed 'Topology' section for TTL Type (Need "
            "PixelOffset, PixelRange)",
            Entry));
      }
    }

    if (MonitorType == CbmType::IBM) {

      try {
        param1 = Module["MaxTofBin"].get<int>();
        param2 = Module["BinCount"].get<int>();
      } catch (...) {
        errorExit(fmt::format(
            "Entry: {}, Malformed 'Topology' section for IBM Type (Need "
            "MaxTofBin, BinCount)",
            Entry));
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
