// Copyright (C) 2018-2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file AbstractClusterer.cpp
/// \brief AbstractClusterer class implementation
///
//===----------------------------------------------------------------------===//

#include <common/reduction/clustering/AbstractClusterer.h>
#include <fmt/format.h>

#include <common/Trace.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

char *GreedyClusterStorage::s_MemBegin = (char*)malloc(1024 * 1024 * 1024);
char *GreedyClusterStorage::s_MemEnd = s_MemBegin + (1024 * 1024 * 1024);

ClusterPoolStorage::AllocConfig::PoolType* ClusterPoolStorage::s_Pool =
    new AllocConfig::PoolType();

// Note: We purposefully leak the storage, since the EFU doesn't guaranteed that
// all memory is freed in the proper order (or at all).
PoolAllocator<ClusterPoolStorage::AllocConfig>
    ClusterPoolStorage::s_Alloc(*ClusterPoolStorage::s_Pool);

std::string to_string(const ClusterContainer &container,
                      const std::string &prepend, bool verbose) {
  if (container.empty())
    return {};
  std::stringstream ss;
  for (const auto &cluster : container) {
    ss << prepend << cluster.to_string(prepend, verbose) << "\n";
  }
  return ss.str();
}

std::string AbstractClusterer::status(const std::string &prepend,
                                      bool verbose) const {
  std::stringstream ss;
  ss << prepend
     << fmt::format("total_cluster_count: {}\n", stats_cluster_count);
  if (!clusters.empty()) {
    ss << prepend << "Current clusters [" << clusters.size() << "]:\n";
    ss << to_string(clusters, prepend + "  ", verbose);
  }
  return ss.str();
}

bool AbstractClusterer::empty() const { return clusters.empty(); }

void AbstractClusterer::stash_cluster(Cluster &cluster) {
  XTRACE(CLUSTER, DEB, status("", true).c_str());

  clusters.emplace_back(std::move(cluster));
  stats_cluster_count++;
}
