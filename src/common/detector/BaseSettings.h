// Copyright (C) 2023 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief EFU BaseSettings
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>
#include <string>
#include <vector>

// All settings should be initialized.
// clang-format off
struct BaseSettings {
  std::string   ConfigFile      {""};
  std::string   DetectorName    {""};
  ///\brief Connection/socket settings
  std::string   DetectorAddress {"0.0.0.0"};
  uint16_t DetectorPort         {9000};
  uint16_t CommandServerPort    {8888}; /// \todo make same as detector port
  int32_t  ReceiveMaxBytes      {9000}; // Jumbo frame support
  int32_t  RxSocketBufferSize   {2000000}; // bytes
  int32_t  TxSocketBufferSize   {2000000}; // bytes
  int32_t  SocketRxTimeoutUS    {100}; // wait only 100us for data on socket
  /// /brief Monitoring
  uint32_t MonitorPeriod        {1000};  // start capturing every 1000 packets
  uint32_t MonitorSamples       {2};     // capture 2 consecutive packets
  ///\brief Kafka settings
  std::string   KafkaConfigFile {""}; // use default
  std::string   KafkaBroker     {"localhost:9092"};
  std::string   KafkaTopic      {""};
  std::string   KafkaDebugTopic {""};
  ///\brief Graphite setting
  std::string   GraphitePrefix  {""};
  std::string   GraphiteRegion  {"0"};
  std::string   GraphiteAddress {"127.0.0.1"};
  uint16_t      GraphitePort    {2003};
  ///\brief Application behavior
  uint64_t      UpdateIntervalSec    {1};
  uint32_t      StopAfterSec    {0xffffffffU};
  bool          NoHwCheck       {false};
  std::vector<std::string>  Interfaces {};
  std::string   CalibFile       {""};
  ///\brief module specific configurations
  // perfgen
  bool          TestImage            {false};
  uint32_t TestImageUSleep         {10};
  uint32_t TestImageEventsPerPulse {500};
  // legacy module support
  bool          MultibladeAlignment{false};
};
// clang-format on
