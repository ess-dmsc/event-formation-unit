/** Copyright (C) 2016 - 2018 European Spallation Source ERIC */

#include <common/Log.h>
#include <common/NewStats.h>

NewStats::NewStats() {}

NewStats::~NewStats() {}

NewStats::NewStats(std::string pre) { setPrefix(pre); }

int NewStats::create(std::string statname, int64_t &counter) {
  LOG(Sev::Info, "Adding stat {}", statname);
  counter = 0;
  auto pfname = prefix + statname;
  for (auto s : stats) {
    if (s.name == pfname or &counter == &s.counter) {
      LOG(Sev::Error, "Duplicate name or counter address for {}", statname);
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

void NewStats::setPrefix(std::string StatsPrefix) {
  if (StatsPrefix.size() > 0) {
    const char LastChar = StatsPrefix.back();
    const char PointChar = '.';
    if (LastChar != PointChar) {
      prefix = StatsPrefix + PointChar;
      return;
    }
  }
  prefix = StatsPrefix;
}
