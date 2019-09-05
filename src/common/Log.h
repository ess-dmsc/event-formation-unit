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

using Sev = Log::Severity;

inline int SevToInt(Sev Level) { // Force the use of the correct type
  return static_cast<int>(Level);
}

#define LOG(Group, Severity, Format, ...)   \
  ((TRC_MASK & TRC_G_##Group)               \
    ?  Log::Msg(SevToInt(Severity), fmt::format(Format, ##__VA_ARGS__), {{"file", std::string(__FILE__)}, {"line", std::int64_t(__LINE__)}}) \
    : (void)0)

