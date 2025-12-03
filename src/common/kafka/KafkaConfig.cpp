// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/kafka/KafkaConfig.h>
#include <iostream>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

KafkaConfig::KafkaConfig(const std::string &KafkaConfigFile) {
  if (KafkaConfigFile == "") {
    CfgParms = DefaultConfig;
    XTRACE(INIT, ALW, "KAFKA CONFIG - DEFAULT");
    return;
  }
  XTRACE(INIT, ALW, "KAFKA CONFIG FROM FILE");
  nlohmann::json root = Json::fromFile(KafkaConfigFile);

  try {
    nlohmann::json KafkaParms = root["KafkaParms"];

    for (const auto &Parm : KafkaParms) {
      std::map<std::string, std::string> MyMap = Parm;
      for (auto it = MyMap.begin(); it != MyMap.end(); it++) {
        std::pair<std::string, std::string> CfgPair{it->first, it->second};
        CfgParms.push_back(CfgPair);
      }
    }

  } catch (...) {
    LOG(INIT, Sev::Error, "Kafka JSON config - error: Invalid Json file: {}",
        KafkaConfigFile);
    throw std::runtime_error("Invalid Json file for Kafka config");
  }
}

std::string KafkaConfig::getCfgParmValStr(const std::string &Key) const {
  for (const auto &Parm : CfgParms) {
    if (Parm.first == Key) {
      return Parm.second;
    }
  }
  return "";
}

int KafkaConfig::getCfgParmValInt(const std::string &Key) const {
  std::string Val = getCfgParmValStr(Key);
  if (Val.empty()) {
    return 0;
  }
  try {
    return std::stoi(Val);
  } catch (...) {
    return 0;
  }
}
