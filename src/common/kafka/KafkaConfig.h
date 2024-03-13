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
#include <utility>
#include <vector>

class KafkaConfig {
public:
  ///\brief Load Kafka configuration from file
  ///\param KafkaConfigFile
  KafkaConfig(std::string KafkaConfigFile);

public:
  // Parameters obtained from JSON config file
  std::vector<std::pair<std::string, std::string>> CfgParms;

  static inline std::vector<std::pair<std::string, std::string>> DefaultConfig{
      {"message.max.bytes", "10000000"},
      {"message.copy.max.bytes", "10000000"},
      {"queue.buffering.max.ms", "100"},
      {"statistics.interval.ms", "1000"},
      {"api.version.request", "true"}};
};
