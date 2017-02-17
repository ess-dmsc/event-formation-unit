/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/NewStats.h>

NewStats::NewStats() {}

NewStats::~NewStats() {
  for (auto s : stats) {
    delete s;
  }
  stats.clear();
}

NewStats::NewStats(std::string pre) : prefix(pre) {}

int NewStats::create(std::string statname, int64_t *counter) {
  auto pfname = prefix + statname;
  for (auto s : stats) {
    if (s->name == pfname || s->counter == counter) {
      return -1;
    }
  }
  stats.push_back(new StatTuple(pfname, counter));
  return 0;
}

size_t NewStats::size() { return stats.size(); }

std::string &NewStats::name(size_t index) {
  if (index > stats.size() || index < 1) {
    return nostat;
  }
  return stats[index - 1]->name;
}

int64_t NewStats::value(size_t index) {
  if (index > stats.size() || index < 1) {
    return -1;
  }
  return *stats[index - 1]->counter;
}
