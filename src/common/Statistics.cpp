// Copyright (C) 2016-2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of named stat counters
///
//===----------------------------------------------------------------------===//

#include <common/Statistics.h>
#include <common/debug/Log.h>

int Statistics::create(std::string StatName, int64_t &Value) {
  LOG(UTILS, Sev::Info, "Adding stat {}", StatName);
  Value = 0; // all counters are cleared
  auto FullStatName = prefix + StatName;
  for (auto s : stats) {
    if (s.StatName == FullStatName or &Value == &s.StatValue) {
      LOG(UTILS, Sev::Error, "Duplicate StatName or Value address for {}",
          StatName);
      return -1;
    }
  }
  stats.push_back(StatTuple(FullStatName, Value));
  return 0;
}

size_t Statistics::size() { return stats.size(); }

std::string &Statistics::name(size_t Index) {
  if (Index > stats.size() || Index < 1) {
    return nostat;
  }
  return stats.at(Index - 1).StatName;
}

int64_t Statistics::value(size_t Index) {
  if (Index > stats.size() || Index < 1) {
    return -1;
  }
  return stats.at(Index - 1).StatValue;
}

int64_t Statistics::valueByName(std::string name) {
  for (const auto & stat : stats) {
    if (stat.StatName.find(name) != std::string::npos) {
      return stat.StatValue;
    }
  }
  return -1;
}

void Statistics::setPrefix(std::string StatsPrefix, std::string StatsRegion) {
  if (StatsPrefix.size() > 0) {
    prefix = StatsPrefix;
    const char LastChar = StatsPrefix.back();

    if (LastChar != PointChar) {
      prefix = prefix + PointChar;
    }
  }

  if (StatsRegion.size() > 0) {
    prefix = prefix + StatsRegion;
    const char LastChar = StatsRegion.back();

    if (LastChar != PointChar) {
      prefix = prefix + PointChar;
    }
  }
}
