/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#pragma once

#include <NMX/EventNMX.h>
#include <NMX/Eventlet.h>
#include <vector>
#include <map>

class Clusterer {
public:
  Clusterer(uint64_t min_time_span);

  void insert(const std::vector<Eventlet>& events);

  /** @todo Martin document */
  bool event_ready() const;

  /** @todo Martin document */
  EventNMX get();

private:
  std::multimap<uint64_t, Eventlet> backlog_; /**< @todo Martin document */
  uint64_t min_time_span_ {1};
};
