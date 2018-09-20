/** Copyright (C) 2018 European Spallation Source ERIC */

#pragma once

#include <fmt/format.h>
#include <cstdint>
#include "TraceGroups.h"
#include <libgen.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <graylog_logger/GraylogInterface.hpp>
#include <graylog_logger/FileInterface.hpp>
#include <graylog_logger/ConsoleInterface.hpp>
#include <graylog_logger/Log.hpp>
#pragma GCC diagnostic pop

#pragma GCC system_header

enum class Sev : int {
  Emergency = 0,
  Alert = 1,
  Critical = 2,
  Error = 3,
  Warning = 4,
  Notice = 5,
  Info = 6,
  Debug = 7,
  };

  inline int SevToInt(Sev Level) { // Force the use of the correct type
    return static_cast<int>(Level);
  }

#define LOG(Group, Severity, Format, ...) \
  if ((TRC_MASK & TRC_G_##Group) != 0) {\
    Log::Msg(SevToInt(Severity), fmt::format(Format, ##__VA_ARGS__), {{"file", std::string(__FILE__)}, {"line", std::int64_t(__LINE__)}}); \
  }

// #define XTRACE(Group, Level, Format, ...)                                   \
// (void)(((TRC_L_##Level <= TRC_LEVEL) && (TRC_MASK & TRC_G_##Group))          \
// ? Trace(__LINE__, __FILE__, #Group, #Level, Format,\
// ##__VA_ARGS__) \
// : 0)
