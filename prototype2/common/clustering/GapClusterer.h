/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/clustering/AbstractClusterer.h>

class GapClusterer : public AbstractClusterer {
public:
  GapClusterer(uint64_t maxTimeGap, uint16_t maxStripGap, size_t minClusterSize);
  ~GapClusterer() {}

  void cluster(const HitContainer &hits) override;

private:
  uint64_t pMaxTimeGap;
  uint16_t pMaxStripGap;
  size_t pMinClusterSize;

  void cluster_by_time(const HitContainer &oldHits);
  void cluster_by_coordinate(HitContainer &cluster);
  void stash_cluster(Cluster &plane);
};
