/// Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <gdgem/clustering/AbstractClusterer.h>

class DoroClusterer : public AbstractClusterer {
public:
  DoroClusterer(double maxTimeGap, uint16_t maxStripGap, size_t minClusterSize);
  ~DoroClusterer() {}

  void cluster(const HitContainer &hits) override;

private:
  double pMaxTimeGap;
  uint16_t pMaxStripGap;
  size_t pMinClusterSize;

  void cluster_by_time(const HitContainer &oldHits);
  void cluster_by_strip(HitContainer &cluster);
  void stash_cluster(Cluster &plane);
};
