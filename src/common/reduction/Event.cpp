/** Copyright (C) 2016, 2017 European Spallation Source ERIC */
//===----------------------------------------------------------------------===//
///
/// \file Event.cpp
/// \brief Event class implementation
///
//===----------------------------------------------------------------------===//

#include <cmath>
#include <common/debug/Trace.h>
#include <common/reduction/Event.h>
#include <fmt/format.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

Event::Event(uint8_t plane1, uint8_t plane2)
    : PlaneA_(plane1), PlaneB_(plane2) {}

uint8_t Event::PlaneA() const { return PlaneA_; }

uint8_t Event::PlaneB() const { return PlaneB_; }

void Event::insert(const Hit &e) {
  if (e.plane == PlaneA_) {
    ClusterA.insert(e);
  } else if (e.plane == PlaneB_) {
    ClusterB.insert(e);
  }
}

size_t Event::totalHitCount() const {
  return ClusterA.hitCount() + ClusterB.hitCount();
}

void Event::merge(Cluster &cluster) {
  if (cluster.plane() == PlaneA_) {
    ClusterA.merge(cluster);
  } else if (cluster.plane() == PlaneB_) {
    ClusterB.merge(cluster);
  }
  XTRACE(EVENT, DEB, "merge() ClusterA size %u. ClusterB size %u",
         ClusterA.hitCount(), ClusterB.hitCount());
}

void Event::clear() {
  ClusterA.clear();
  ClusterB.clear();
}

bool Event::empty() const { return ClusterA.empty() && ClusterB.empty(); }

bool Event::both_planes() const {
  return !ClusterA.empty() && !ClusterB.empty();
}

uint64_t Event::timeEnd() const {
  if (ClusterA.empty())
    return ClusterB.timeEnd();
  if (ClusterB.empty())
    return ClusterA.timeEnd();
  return std::max(ClusterA.timeEnd(), ClusterB.timeEnd());
}

uint64_t Event::timeStart() const {
  if (ClusterA.empty())
    return ClusterB.timeStart();
  if (ClusterB.empty())
    return ClusterA.timeStart();
  return std::min(ClusterA.timeStart(), ClusterB.timeStart());
}

uint64_t Event::timeSpan() const {
  if (empty())
    return 0;
  return (timeEnd() - timeStart()) + 1ul;
}

uint64_t Event::timeOverlap(const Cluster &other) const {
  if (empty() || other.empty()) {
    return 0;
  }
  auto latest_start = std::max(other.timeStart(), timeStart());
  auto earliest_end = std::min(other.timeEnd(), timeEnd());

  if (latest_start > earliest_end) {
    XTRACE(EVENT, DEB, "no time overlap");
    return 0;
  }
  return (earliest_end - latest_start) + 1ul;
}

uint64_t Event::timeGap(const Cluster &other) const {
  if (empty() || other.empty()) {
    /// In case of two empty clusters time gap ought to be undefined or "inf"
    /// Returning max value of the used type, but throwing an exception
    /// could also be an option
    return std::numeric_limits<uint64_t>::max();
  }
  auto latest_start = std::max(other.timeStart(), timeStart());
  auto earliest_end = std::min(other.timeEnd(), timeEnd());

  if (latest_start <= earliest_end) {
    XTRACE(EVENT, DEB, "no time gap");
    return 0;
  }
  return (latest_start - earliest_end);
}

std::string Event::to_string(const std::string &prepend, bool verbose) const {
  std::stringstream ss;
  ss << fmt::format("Event planes({}{},{}{})", PlaneA_,
                    (ClusterA.empty() ? "" : "*"), PlaneB_,
                    (ClusterB.empty() ? "" : "*"));
  if (!ClusterA.empty())
    ss << "\n"
       << prepend
       << "  PlaneA:  " << ClusterA.to_string(prepend + "  ", verbose);
  if (!ClusterB.empty())
    ss << "\n"
       << prepend
       << "  PlaneB:  " << ClusterB.to_string(prepend + "  ", verbose);
  return ss.str();
}

std::string Event::visualize(const std::string &prepend,
                             uint8_t downsample_time,
                             uint8_t downsample_coords) const {
  std::stringstream ss;
  if (!ClusterA.empty())
    ss << prepend << "  PlaneA:\n"
       << ClusterA.visualize(prepend + "  ", downsample_time,
                             downsample_coords);
  if (!ClusterB.empty())
    ss << prepend << "  PlaneB:\n"
       << ClusterB.visualize(prepend + "  ", downsample_time,
                             downsample_coords);
  return ss.str();
}
