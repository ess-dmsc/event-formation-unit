// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of named stat counters
///
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <common/Statistics.h>
#include <common/debug/Log.h>

Statistics::Statistics(const std::string &StatsPrefix,
                       const std::string &StatsRegion) {
  setPrefix(StatsPrefix, StatsRegion);
  LOG(UTILS, Sev::Info, "Statistics initialized with prefix: {}",
      DefaultPrefix);
}

int Statistics::create(const std::string &StatName, int64_t &Value,
                       const std::string &Prefix) {
  LOG(UTILS, Sev::Info, "Adding stat {}", StatName);
  Value = 0; // all counters are cleared

  std::string effectivePrefix = Prefix.empty() ? DefaultPrefix : Prefix;

  // Check for duplicates using thread-safe operations
  bool isDuplicate = stats.any_of([&](const StatTuple& s) {
    return (s.StatName == StatName && s.StatPrefix == effectivePrefix) ||
           &Value == &s.StatValue;
  });

  if (isDuplicate) {
    LOG(UTILS, Sev::Error,
        "Duplicate StatName/Prefix combination or Value address for {}",
        StatName);
    return -1;
  }

  stats.emplace_back(StatName, Value, effectivePrefix);
  XTRACE(UTILS, DEB, "Created stat %s with prefix %s and value address %p",
         StatName.c_str(), effectivePrefix.c_str(), &Value);

  return 0;
}

size_t Statistics::size() const { return stats.size(); }

const std::string &Statistics::getStatName(size_t Index) const {
  if (Index > stats.size() || Index < 1) {
    return nostat;
  }
  return stats.at(Index - 1).StatName;
}

bool Statistics::setStatName(size_t Index, const std::string &Name) {
  if (Index > stats.size() || Index < 1) {
    return false;
  }
  stats.at(Index - 1).StatName = Name;
  return true;
}

bool Statistics::setStatName(size_t Index, std::string &&Name) {
  if (Index > stats.size() || Index < 1) {
    return false;
  }
  stats.at(Index - 1).StatName = std::move(Name);
  return true;
}

const std::string &Statistics::getStatPrefix(size_t Index) const {
  if (Index > stats.size() || Index < 1) {
    return nostat;
  }
  return stats.at(Index - 1).StatPrefix;
}

bool Statistics::setStatPrefix(size_t Index, const std::string &Prefix) {
  if (Index > stats.size() || Index < 1) {
    return false;
  }
  stats.at(Index - 1).StatPrefix = Prefix;
  return true;
}

bool Statistics::setStatPrefix(size_t Index, std::string &&Prefix) {
  if (Index > stats.size() || Index < 1) {
    return false;
  }
  stats.at(Index - 1).StatPrefix = std::move(Prefix);
  return true;
}

std::string Statistics::getFullName(size_t Index) const {
  if (Index > stats.size() || Index < 1) {
    return nostat;
  }

  const StatTuple &stat = stats.at(Index - 1);
  return stat.StatPrefix + stat.StatName;
}

int64_t Statistics::getValue(size_t Index) const {
  if (Index > stats.size() || Index < 1) {
    return -1;
  }
  return stats.at(Index - 1).StatValue;
}

int64_t Statistics::getValueByName(const std::string_view &name,
                                   const std::string &Prefix) const {

  std::string effectivePrefix = Prefix.empty() ? DefaultPrefix : Prefix;

  const StatTuple* found = stats.find_if([&](const StatTuple& stat) {
    return stat.StatName == name && stat.StatPrefix == effectivePrefix;
  });

  return found ? found->StatValue : -1;
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
