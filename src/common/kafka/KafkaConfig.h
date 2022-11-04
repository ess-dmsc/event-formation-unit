// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Read kafka configuration from file
//===----------------------------------------------------------------------===//

#pragma once

#include <common/JsonFile.h>
#include <string>
#include <vector>
#include <utility>

class KafkaConfig {
public:
  // Load Kafka configuration from file
  KafkaConfig(std::string KafkaConfigFile);

public:
  // Parameters obtained from JSON config file
  std::vector<std::pair<std::string, std::string>> CfgParms;

  static inline std::vector<std::pair<std::string,std::string>> DefaultConfig {
    {"message.max.bytes", "10000000"},
    {"fetch.message.max.bytes", "10000000"},
    {"message.copy.max.bytes", "10000000"},
    {"queue.buffering.max.ms", "100"},
    {"api.version.request", "true"}
  };
};
