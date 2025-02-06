// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Collection of Graylog methods for EFU
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Log.h>
#include <string>

class Graylog {
public:
  ~Graylog() { EmptyGraylogMessageQueue(); };

  static std::string ConsoleFormatter(const Log::LogMessage &Msg);

  static std::string FileFormatter(const Log::LogMessage &Msg);

  void EmptyGraylogMessageQueue();

  void AddLoghandlerForNetwork(const std::string &DetectorName, const std::string &FileName,
                               int LogLevel, const std::string &Address, int Port);
};
