// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Monitor config
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/JsonFile.h>
#include <common/debug/Trace.h>
#include <common/readout/ess/Parser.h>
#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace TTLMonitor {

class Config {
public:

  Config(std::string ConfigFile) : FileName(ConfigFile) {};

  Config() {};

  // load file into json object and apply
  void loadAndApply();

  // Apply the loaded json file
  void apply();

  // Parameters (eventually) obtained from JSON config file
  struct {
    std::string TTLMonitorTopic{"ttlmonitor_beam_monitor"};
    uint8_t TypeSubType{ESSReadout::Parser::TTLMonitor};
    uint32_t MaxTOFNS{1'000'000'000};
    uint32_t MaxPulseTimeNS{1'000'000'000}; // 1 second
  } Parms;

  std::string FileName{""};
  nlohmann::json root;
};

} // namespace TTLMonitor