// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
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

KafkaConfig::KafkaConfig(std::string KafkaConfigFile) {
  if (KafkaConfigFile == "") {
    CfgParms = DefaultConfig;
    XTRACE(INIT, ALW, "KAFKA CONFIG - DEFAULT");
    return;
  }
  XTRACE(INIT, ALW, "KAFKA CONFIG FROM FILE");
  nlohmann::json root = from_json_file(KafkaConfigFile);

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
    return;
  }
}
