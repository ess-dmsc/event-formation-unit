/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/NewStats.h>

NewStats::NewStats() {}

int NewStats::create(std::string statname, int64_t * counter) {
 for (auto & s : stats) {
   if (s->name == statname || s->counter == counter) {
     return -1;
   }
 }

 auto stat = new StatTuple();
 stat->name = statname;
 stat->counter = counter;
 stats.push_back(stat);
 return 0;
}

size_t NewStats::size() {return stats.size(); }

std::string & NewStats::name(size_t index) {
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
