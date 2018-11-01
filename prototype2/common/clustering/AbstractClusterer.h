/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <list>
#include <common/clustering/Cluster.h>

using HitContainer = std::vector<Hit>;

/// \todo pick a faster data structure
using ClusterList = std::list<Cluster>;

class AbstractClusterer {
public:
  AbstractClusterer() {}
  virtual ~AbstractClusterer() {}

  virtual void cluster(const HitContainer &hits) = 0;
  virtual void flush() = 0;

  bool empty() const
  {
    return clusters.empty();
  }

  uint8_t plane{0};
  size_t stats_cluster_count{0};
  ClusterList clusters;
};
