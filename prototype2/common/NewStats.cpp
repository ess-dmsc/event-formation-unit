/** Copyright (C) 2016 - 2018 European Spallation Source ERIC */

#include <common/Log.h>
#include <common/NewStats.h>

int NewStats::create(std::string statname, int64_t &counter) {
  LOG(UTILS, Sev::Info, "Adding stat {}", statname);
  counter = 0;
  auto pfname = prefix + statname;
  for (auto s : stats) {
    if (s.name == pfname or &counter == &s.counter) {
      LOG(UTILS, Sev::Error, "Duplicate name or counter address for {}", statname);
      return -1;
    }
  }
  stats.push_back(StatTuple(pfname, counter));
  return 0;
}

size_t NewStats::size() { return stats.size(); }

std::string &NewStats::name(size_t index) {
  if (index > stats.size() || index < 1) {
    return nostat;
  }
  return stats.at(index - 1).name;
}

int64_t NewStats::value(size_t index) {
  if (index > stats.size() || index < 1) {
    return -1;
  }
  return stats.at(index - 1).counter;
}

void NewStats::setPrefix(std::string StatsPrefix, std::string StatsRegion) {
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
