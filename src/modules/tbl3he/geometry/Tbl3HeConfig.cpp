// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <tbl3he/geometry/Tbl3HeConfig.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

namespace Caen {

///
Tbl3HeConfig::Tbl3HeConfig() {}

Tbl3HeConfig::Tbl3HeConfig(std::string ConfigFile) : ConfigFileName(ConfigFile) {
  XTRACE(INIT, DEB, "Loading json file");
  root = from_json_file(ConfigFile);
}

void Tbl3HeConfig::parseConfig() {

  json_check_keys("Mandatory keys", root, {"Detector", "Resolution", "NumOfFENs", "Topology"});

  Parms.InstrumentName = root["Detector"].get<std::string>();
  if (Parms.InstrumentName != "tbl3he") {
    LOG(INIT, Sev::Error, "Invalid instrument name ({}) for tbl3he", Parms.InstrumentName);
    throw std::runtime_error("InstrumentName != 'tblhe'");
  }

  try {
    // Assumed the same for all straws in all banks
    Parms.Resolution = root["Resolution"].get<int>();
    XTRACE(INIT, DEB, "Resolution %d", Parms.Resolution);

    Parms.MaxPulseTimeNS = root["MaxPulseTimeNS"].get<unsigned int>();
    LOG(INIT, Sev::Info, "MaxPulseTimeNS: {}", Parms.MaxPulseTimeNS);

    Parms.MaxTOFNS = root["MaxTOFNS"].get<unsigned int>();
    LOG(INIT, Sev::Info, "MaxTOFNS: {}", Parms.MaxTOFNS);


    TopologyMapPtr.reset(new HashMap2D<Topology>(Parms.NumOfFENs));
    // Run through the Topology section
    auto Configs = root["Topology"];

    for (auto & elt : Configs) {
      printf(".\n");
      int Ring = elt["Ring"].get<int>();
      if ((Ring < 0) or (Ring >= Parms.NumRings)) {
        XTRACE(INIT, WAR, "Invalid ring: %d", Ring);
        continue;
      }
      int FEN = elt["FEN"].get<unsigned int>();
      int Bank = elt["Bank"].get<unsigned int>();
      //BankTopology.add(Ring, FEN, )
      printf("Use Hashmap for Ring %d, FEN %d, Bank %d\n", Ring, FEN, Bank);
      //
    }
    printf("ZZ ZZ\n");
    //XTRACE(INIT, ALW, "Rings configured: %d", Parms.ConfiguredRings);




  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Json file: {}",
        ConfigFileName);
    throw std::runtime_error("Invalid Json file");
  }
}

} // namespace Caen
