/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <list>
#include <deque>
#include <common/clustering/Cluster.h>

using HitContainer = std::vector<Hit>;
using ClusterContainer = std::list<Cluster>;

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

  ClusterContainer clusters; ///< clustered hits
  size_t stats_cluster_count{0}; ///< cumulative number of clusters produced
};
