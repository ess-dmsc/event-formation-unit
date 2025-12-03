// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
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
  KafkaConfig(const std::string &KafkaConfigFile);

  /// \brief Returns the value of a configuration parameter
  /// \param Key The configuration key
  /// \return The configuration value
  std::string getCfgParmValStr(const std::string &Key) const;

  /// \brief Returns the value of a configuration parameter as an integer
  /// \param Key The configuration key
  /// \return The configuration value as an integer
  int getCfgParmValInt(const std::string &Key) const;

  // Parameters obtained from JSON config file
  std::vector<std::pair<std::string, std::string>> CfgParms;

  static inline std::vector<std::pair<std::string, std::string>> DefaultConfig{
      {"message.max.bytes", "10000000"},      // max size of a msg (10MB)
      {"message.copy.max.bytes", "10000000"}, // max bytes of a msg to copy over into kafka buffer (10MB)
      {"queue.buffering.max.kbytes", "1000000"}, // max kbytes in queue (1GB)
      {"queue.buffering.max.messages", "10000"}, // max messages in queue
      {"queue.buffering.max.ms", "100"},
      {"statistics.interval.ms", "1000"},    // every sec read out kafka statistic
      {"message.timeout.ms", "600000"},      // 10 minutes to keep msg memory
      {"api.version.request", "true"}};
};
