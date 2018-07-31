/** Copyright (C) 2018 European Spallation Source ERIC */

#pragma once

#include <fmt/format.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <graylog_logger/GraylogInterface.hpp>
#include <graylog_logger/FileInterface.hpp>
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

#define LOG(Severity, Format, ...) Log::Msg(int(Severity), fmt::format(Format, ##__VA_ARGS__))
