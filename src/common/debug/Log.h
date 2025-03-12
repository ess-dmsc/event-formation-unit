// Copyright (C) 2018 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief LOG macros using ESS implementation of Graylog logger
///
/// See https://github.com/ess-dmsc/graylog-logger
/// Trace groups can be compile time excluded.
//===----------------------------------------------------------------------===//

#pragma once

#include "TraceGroups.h"
#include <cstdint>
#include <fmt/format.h>
#include <libgen.h>
#include <trompeloeil.hpp>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wpedantic"
#include <graylog_logger/ConsoleInterface.hpp>
#include <graylog_logger/FileInterface.hpp>
#include <graylog_logger/GraylogInterface.hpp>
#include <graylog_logger/Log.hpp>
#pragma GCC diagnostic pop

#pragma GCC system_header

using Sev = Log::Severity;

inline int SevToInt(Sev Level) { // Force the use of the correct type
  return static_cast<int>(Level);
}

#ifdef UNIT_TEST

/// \brief MockLogger is a mock class for the logger. It provides a log method
/// that can be mocked to check if the logger is called with the correct
/// parameters.
class MockLogger {
public:
  virtual void log(const std::string category, const std::string message) = 0;
  
  /// \brief Function to log messages using the mock logger. If the MockLogger
  /// instance is not set, the log message is ignored.
  /// \param category The category of the log message
  /// \param message The message to log
  static void mockLogFunction(const std::string category, const std::string message);

  static void registerMockLogger(MockLogger *logger) { currentMockLogger = logger; }
  
private:
  static MockLogger *currentMockLogger;
};

#define LOG(Group, Severity, Format, ...)                                      \
  MockLogger::mockLogFunction(#Group, fmt::format(Format, ##__VA_ARGS__))
#else
#define LOG(Group, Severity, Format, ...)                                      \
  ((TRC_MASK & TRC_G_##Group)                                                  \
       ? Log::Msg(SevToInt(Severity), fmt::format(Format, ##__VA_ARGS__),      \
                  {{"file", std::string(__FILE__)},                            \
                   {"line", std::int64_t(__LINE__)}})                          \
       : (void)0)
#endif
