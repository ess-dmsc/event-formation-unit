/** Copyright (C) 2016, 2017 European Spallation Source ERIC */
//===----------------------------------------------------------------------===//
///
/// \file Event.cpp
/// \brief Event class implementation
///
//===----------------------------------------------------------------------===//

#include <common/clustering/Event.h>
#include <common/Trace.h>
#include <fmt/format.h>
#include <cmath>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

Event::Event(uint8_t plane1, uint8_t plane2)
    : PlaneA_(plane1), PlaneB_(plane2) {}

uint8_t Event::PlaneA() const {
  return PlaneA_;
}

uint8_t Event::PlaneB() const {
  return PlaneB_;
}

void Event::insert(const Hit &e) {
  if (e.plane == PlaneA_) {
    ClusterA.insert(e);
  } else if (e.plane == PlaneB_) {
    ClusterB.insert(e);
  }
}

size_t Event::total_hit_count() const {
  return ClusterA.hit_count() + ClusterB.hit_count();
}

void Event::merge(Cluster &cluster) {
  if (cluster.plane() == PlaneA_) {
    ClusterA.merge(cluster);
  } else if (cluster.plane() == PlaneB_) {
    ClusterB.merge(cluster);
  }
  XTRACE(EVENT, DEB, "merge() ClusterA size %u. ClusterB size %u",
         ClusterA.hit_count(), ClusterB.hit_count());
}

void Event::clear() {
  ClusterA.clear();
  ClusterB.clear();
}

bool Event::empty() const {
  return ClusterA.empty() && ClusterB.empty();
}

bool Event::both_planes() const {
  return !ClusterA.empty() && !ClusterB.empty();
}

uint64_t Event::time_end() const {
  if (ClusterA.empty())
    return ClusterB.time_end();
  if (ClusterB.empty())
    return ClusterA.time_end();
  return std::max(ClusterA.time_end(), ClusterB.time_end());
}

uint64_t Event::time_start() const {
  if (ClusterA.empty())
    return ClusterB.time_start();
  if (ClusterB.empty())
    return ClusterA.time_start();
  return std::min(ClusterA.time_start(), ClusterB.time_start());
}

uint64_t Event::time_span() const {
  if (empty())
    return 0;
  return (time_end() - time_start()) + 1ul;
}

uint64_t Event::time_overlap(const Cluster &other) const {
  if (empty() || other.empty()) {
    return 0;
  }
  auto latest_start = std::max(other.time_start(), time_start());
  auto earliest_end = std::min(other.time_end(), time_end());

  if (latest_start > earliest_end) {
    XTRACE(EVENT, DEB, "no time overlap");
    return 0;
  }
  return (earliest_end - latest_start) + 1ul;
}

uint64_t Event::time_gap(const Cluster &other) const {
  if (empty() || other.empty()) {
    /// In case of two empty clusters time gap ought to be undefined or "inf"
    /// Returning max value of the used type, but throwing an exception
    /// could also be an option
    return std::numeric_limits<uint64_t>::max();
  }
  auto latest_start = std::max(other.time_start(), time_start());
  auto earliest_end = std::min(other.time_end(), time_end());

  if (latest_start <= earliest_end) {
    XTRACE(EVENT, DEB, "no time gap");
    return 0;
  }
  return (latest_start - earliest_end);
}

std::string Event::to_string(const std::string &prepend, bool verbose) const {
  std::stringstream ss;
  ss << fmt::format("Event planes({}{},{}{})",
                    PlaneA_, (ClusterA.empty() ? "" : "*"),
                    PlaneB_, (ClusterB.empty() ? "" : "*"));
  if (!ClusterA.empty())
    ss << "\n" << prepend << "  PlaneA:  "
       << ClusterA.to_string(prepend + "  ", verbose);
  if (!ClusterB.empty())
    ss << "\n" << prepend << "  PlaneB:  "
       << ClusterB.to_string(prepend + "  ", verbose);
  return ss.str();
}

std::string Event::visualize(const std::string &prepend,
                             uint8_t downsample_time,
                             uint8_t downsample_coords) const {
  std::stringstream ss;
  if (!ClusterA.empty())
    ss << "\n" << prepend << "  PlaneA:\n"
       << ClusterA.visualize(prepend + "  ", downsample_time, downsample_coords);
  if (!ClusterB.empty())
    ss << "\n" << prepend << "  PlaneB:\n"
       << ClusterB.visualize(prepend + "  ", downsample_time, downsample_coords);
  return ss.str();
}

