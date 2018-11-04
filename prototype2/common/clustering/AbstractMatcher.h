/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */

#pragma once

#include <common/clustering/AbstractClusterer.h>
#include <common/clustering/Event.h>
#include <deque>

class AbstractMatcher {
public:
  AbstractMatcher() {}
  virtual ~AbstractMatcher() {}

  virtual void insert(uint8_t plane, ClusterContainer &c) = 0;
  virtual void flush() = 0;

  std::deque<Event> matched_clusters;
  size_t stats_cluster_count{0};
  /// \todo discarded, other counters?
protected:
  void stash_event(Event& event) {
    matched_clusters.emplace_back(std::move(event));
    stats_cluster_count++;
  }

};
