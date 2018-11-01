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
  GapClusterer(uint64_t TimeGap, uint16_t CoordGap, size_t ClusterSize);
  ~GapClusterer() {}

  void cluster(const HitContainer &hits) override;
  void flush() override;

private:
  uint64_t MaxTimeGap;
  uint16_t MaxCoordGap;
  size_t MinClusterSize;

  HitContainer CurrentTimeCluster;

  void cluster_by_time(const HitContainer &oldHits);
  void cluster_by_coordinate(HitContainer &cluster);
  void stash_cluster(Cluster &plane);
};
