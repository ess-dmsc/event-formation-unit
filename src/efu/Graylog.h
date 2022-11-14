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

  void AddLoghandlerForNetwork(std::string DetectorName, std::string FileName,
                               int LogLevel, std::string Address, int Port);
};
