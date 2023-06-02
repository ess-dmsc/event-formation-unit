// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file Abstract2DClusterer.cpp
/// \brief Abstract2DClusterer class implementation
///
//===----------------------------------------------------------------------===//

#include <common/reduction/clustering/Abstract2DClusterer.h>
#include <fmt/format.h>

#include <common/debug/Trace.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

#if ENABLE_GREEDY_CLUSTER_ALLOCATOR
enum : size_t { Bytes_1GB = 1024 * 1024 * 1024 };
char *GreedyCluster2DStorage::MemBegin = (char *)malloc(Bytes_1GB);
char *GreedyCluster2DStorage::MemEnd = MemBegin + Bytes_1GB;
#else
char *GreedyCluster2DStorage::MemBegin = nullptr;
char *GreedyCluster2DStorage::MemEnd = nullptr;
#endif

Cluster2DPoolStorage::AllocConfig::PoolType *Cluster2DPoolStorage::Pool =
    new AllocConfig::PoolType();

// Note: We purposefully leak the storage, since the EFU doesn't guarantee that
// all memory is freed in the proper order (or at all).
PoolAllocator<Cluster2DPoolStorage::AllocConfig>
    Cluster2DPoolStorage::Alloc(*Cluster2DPoolStorage::Pool);

std::string to_string(const Cluster2DContainer &container,
                      const std::string &prepend, bool verbose) {
  if (container.empty())
    return {};
  std::stringstream ss;
  for (const auto &cluster : container) {
    ss << prepend << cluster.to_string(prepend, verbose) << "\n";
  }
  return ss.str();
}

std::string Abstract2DClusterer::status(const std::string &prepend,
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

bool Abstract2DClusterer::empty() const { return clusters.empty(); }

void Abstract2DClusterer::stash_cluster(Cluster2D &cluster) {
  XTRACE(CLUSTER, DEB, status("", true).c_str());

  clusters.emplace_back(std::move(cluster));
  stats_cluster_count++;
}
