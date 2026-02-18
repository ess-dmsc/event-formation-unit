// Copyright (C) 2022 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <fmt/format.h>
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

  if (Instrument != DetectorName) {
    LOG(INIT, Sev::Error, "Detector name mismatch, expected {}",
        Instrument.toString());
    throw std::runtime_error("Detector name mismatch, expected " +
                             Instrument.toString());
  }

  setMask(LOG);
  assign("MaxPulseTimeDiffNS", CbmParms.MaxPulseTimeDiffNS);
  assign("MaxTOFNS", CbmParms.MaxTOFNS);
  assign("MonitorRing", CbmParms.MonitorRing);
  assign("MaxRing", CbmParms.MaxRing); // Optional, defaults to 11
  assign("NormalizeIBMReadouts", CbmParms.NormalizeIBMReadouts);

  setMask(LOG | CHECK);
  assign("MaxFENId", CbmParms.MaxFENId);

  // Number of FENs must must be 1 even is MaxFENId is 0
  CbmParms.NumOfFENs = CbmParms.MaxFENId + 1;

  TopologyMapPtr.reset(new HashMap2D<Topology>(CbmParms.NumOfFENs));
  if (!root().contains("Topology")) {
    throw std::runtime_error("No 'Topology' section found in the "
                             "configuration. Cannot setup Beam Monitors");
  }
  const nlohmann::json Modules = root()["Topology"];

  // Parse and validate each topology entry
  // user Entry to count actual entry and support error messages with nubmering.
  int Entry{0};
  for (const auto &Module : Modules) {
    // Parse basic fields from JSON
    auto entryData = parseTopologyEntryData(Module);

    // Check for array sizes and duplicate entries
    if (entryData.FEN > CbmParms.MaxFENId) {
      errorExit(fmt::format("Entry: {}, Invalid FEN: {} Max: {}", Entry,
                            entryData.FEN, CbmParms.MaxFENId));
    }

    // Check for duplicate FEN/Channel combination
    if (TopologyMapPtr->isValue(entryData.FEN, entryData.Channel)) {
      errorExit(fmt::format("Entry: {}, Duplicate entry for FEN {} Channel {}",
                            Entry, entryData.FEN, entryData.Channel));
    }

    // Add parameter to the temporary map
    CbmType MonitorType = CbmType::EVENT_0D;
    try {
      MonitorType = CbmType(entryData.Type);
    } catch (...) {
      errorExit(fmt::format("Entry: {}, Invalid Type: {} is not a CBM Type",
                            Entry, entryData.Type));
    }

    // Determine used schema type. However EVENT_2D can only use option EV44
    SchemaType DataSchema = SchemaType::EV44;
    try {
      DataSchema = SchemaType(entryData.Schema);
    } catch (...) {
      errorExit(fmt::format("Entry: {}, Invalid Type: {} is not a Schema Type",
                            Entry, entryData.Schema));
    }

    int param1{0};
    int param2{0};
    int param3{0};
    int param4{0};

    if (MonitorType == CbmType::EVENT_0D) {

      try {
        param1 = Module["PixelOffset"].get<int>();
      } catch (...) {
        errorExit(fmt::format(
            "Entry: {}, Malformed 'Topology' section for {} Type (Need "
            "PixelOffset)",
            Entry, MonitorType.toString()));
      }
    }

    if (MonitorType == CbmType::EVENT_2D) {
      try {
        param1 = Module["Width"].get<int>();
        param2 = Module["Height"].get<int>();
      } catch (...) {
        errorExit(fmt::format(
            "Entry: {}, Malformed 'Topology' section for {} Type (Need "
            "Width, Height)",
            Entry, MonitorType.toString()));
      }
      // Maximum allowed value for width and height
      const uint16_t maxValue = std::numeric_limits<uint16_t>::max();
      if ((param1 < 0) || (param1 > maxValue)) {
        errorExit(fmt::format("Entry: {} for {} Type "
                              " valid width values {} - {}, Read Value {}",
                              Entry, MonitorType.toString(), 0, maxValue,
                              param1));
      }
      if ((param2 < 0) || (param2 > maxValue)) {
        errorExit(fmt::format("Entry: {} for {} Type "
                              "valid height values {} - {}, Read Value {}",
                              Entry, MonitorType.toString(), 0, maxValue,
                              param2));
      }
      if (DataSchema != SchemaType::EV44) {
        errorExit(fmt::format("Entry: {} for {} Schema only EV44 is allowed",
                              Entry, DataSchema.toString()));
      }
    }

    if (MonitorType == CbmType::IBM) {

      try {
        param1 = Module["MaxTofBin"].get<int>();
        param2 = Module["BinCount"].get<int>();
        param3 =
            Module.value<int>("AggregatedFrames", CbmParms.AggregatedFrames);
        param4 = Module.value<int>("AggregationMode", CbmParms.AggregationMode);
      } catch (...) {
        errorExit(fmt::format(
            "Entry: {}, Malformed 'Topology' section for {} Type (Need "
            "MaxTofBin, BinCount) optional (AggregatedFrames, AggregationMode)",
            Entry, MonitorType.toString()));
      }

      if ((param4 < 0) || (param4 > 1)) {
        errorExit(
            fmt::format("Entry: {} Malformed 'AggregationMode' for {} Type "
                        "valid values {} - {}, Read Value {}",
                        Entry, MonitorType.toString(), 0,
                        (int)AggregationType::AVG, param4));
      }
    }

    auto topo = std::make_unique<Topology>(
        entryData.FEN, entryData.Channel, entryData.Source, MonitorType,
        DataSchema, param1, param2, param3, param4);
    TopologyMapPtr->add(entryData.FEN, entryData.Channel, topo);

    XTRACE(INIT, ALW, "Entry %02d, FEN %02d, Channel %02d, Source %s Type %s",
           Entry, entryData.FEN, entryData.Channel, entryData.Source.c_str(),
           entryData.Type.c_str());

    // Count the number of valid entries
    Entry++;
  }

  CbmParms.NumberOfMonitors = Entry;
}

TopologyEntryData Config::parseTopologyEntryData(const nlohmann::json &Module) {

  TopologyEntryData data;

  try {
    data.FEN = Module["FEN"].get<int>();
    data.Channel = Module["Channel"].get<int>();
    data.Type = Module["Type"].get<std::string>();
    data.Source = Module["Source"].get<std::string>();
    data.Schema = Module["Schema"].get<std::string>();

  } catch (...) {
    errorExit("Malformed 'Topology' section (Need FEN, Channel, Type, Source, "
              "and Schema)");
  }

  return data;
}

} // namespace cbm
