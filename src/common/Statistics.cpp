// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of named stat counters
///
//===----------------------------------------------------------------------===//

#include "common/debug/Trace.h"
#include <common/Statistics.h>
#include <common/debug/Log.h>

Statistics::Statistics(const std::string &StatsPrefix,
                       const std::string &StatsRegion) {
  setPrefix(StatsPrefix, StatsRegion);
  LOG(UTILS, Sev::Info, "Statistics initialized with prefix: {}", DefaultPrefix);
}

int Statistics::create(const std::string &StatName, int64_t &Value,
                       const std::string &Prefix) {
  LOG(UTILS, Sev::Info, "Adding stat {}", StatName);
  Value = 0; // all counters are cleared

  std::string effectivePrefix = Prefix.empty() ? DefaultPrefix : Prefix;

  for (auto s : stats) {
    // Check if name, prefix pairs or Value address are is a duplicate
    if ((s.StatName == StatName && s.StatPrefix == effectivePrefix) ||
        &Value == &s.StatValue) {
      LOG(UTILS, Sev::Error,
          "Duplicate StatName/Prefix combination or Value address for {}",
          StatName);
      return -1;
    }
  }

  stats.push_back(StatTuple{StatName, Value, effectivePrefix});
  XTRACE(UTILS, DEB, "Created stat %s with prefix %s and value address %p",
         StatName.c_str(), effectivePrefix.c_str(), &Value);

  return 0;
}

size_t Statistics::size() { return stats.size(); }

std::string &Statistics::getStatName(size_t Index) {
  if (Index > stats.size() || Index < 1) {
    return nostat;
  }
  return stats.at(Index - 1).StatName;
}

std::string &Statistics::getStatPrefix(size_t Index) {
  if (Index > stats.size() || Index < 1) {
    return nostat;
  }
  return stats.at(Index - 1).StatPrefix;
}

std::string Statistics::getFullName(size_t Index) {
  if (Index > stats.size() || Index < 1) {
    return nostat;
  }

  StatTuple &stat = stats.at(Index - 1);
  return stat.StatPrefix + stat.StatName;
}

int64_t Statistics::getValue(size_t Index) {
  if (Index > stats.size() || Index < 1) {
    return -1;
  }
  return stats.at(Index - 1).StatValue;
}

int64_t Statistics::getValueByName(const std::string &name,
                                   const std::string &Prefix) {

  std::string effectivePrefix = Prefix.empty() ? DefaultPrefix : Prefix;

  for (const auto &stat : stats) {
    if (stat.StatName == name && stat.StatPrefix == effectivePrefix) {
      return stat.StatValue;
    }
  }
  return -1;
}

void Statistics::setPrefix(const std::string &StatsPrefix,
                           const std::string &StatsRegion) {
  if (StatsPrefix.size() > 0) {
    DefaultPrefix = StatsPrefix;
    const char LastChar = StatsPrefix.back();

    if (LastChar != PointChar) {
      DefaultPrefix = DefaultPrefix + PointChar;
    }
  }

  if (StatsRegion.size() > 0) {
    DefaultPrefix = DefaultPrefix + StatsRegion;
    const char LastChar = StatsRegion.back();

    if (LastChar != PointChar) {
      DefaultPrefix = DefaultPrefix + PointChar;
    }
  }
}
