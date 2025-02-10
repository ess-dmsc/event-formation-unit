// Copyright (C) 2017 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Get current time as a string
///
//===----------------------------------------------------------------------===//

#include <common/time/TimeString.h>
#include <ctime>

std::string timeString() {
  char cStartTime[50];
  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(cStartTime, 50, "%Y%m%d-%H%M%S", timeinfo);
  std::string startTime = cStartTime;
  return startTime;
}
