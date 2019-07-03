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
    : plane1_(plane1), plane2_(plane2) {}

uint8_t Event::plane1() const {
  return plane1_;
}

uint8_t Event::plane2() const {
  return plane2_;
}

void Event::insert(const Hit &e) {
  if (e.plane == plane1_) {
    cluster1.insert(e);
  } else if (e.plane == plane2_) {
    cluster2.insert(e);
  }
}

size_t Event::total_hit_count() const {
  return cluster1.hit_count() + cluster2.hit_count();
}

void Event::merge(Cluster &cluster) {
  if (cluster.plane() == plane1_) {
    cluster1.merge(cluster);
  } else if (cluster.plane() == plane2_) {
    cluster2.merge(cluster);
  }
  XTRACE(EVENT, DEB, "merge() cluster1 size %u. cluster2 size %u", cluster1.hit_count(), cluster2.hit_count());
}

void Event::clear() {
  cluster1.clear();
  cluster2.clear();
}

bool Event::empty() const {
  return cluster1.empty() && cluster2.empty();
}

bool Event::both_planes() const {
  return !cluster1.empty() && !cluster2.empty();
}

uint64_t Event::time_end() const {
  if (cluster1.empty())
    return cluster2.time_end();
  if (cluster2.empty())
    return cluster1.time_end();
  return std::max(cluster1.time_end(), cluster2.time_end());
}

uint64_t Event::time_start() const {
  if (cluster1.empty())
    return cluster2.time_start();
  if (cluster2.empty())
    return cluster1.time_start();
  return std::min(cluster1.time_start(), cluster2.time_start());
}

uint64_t Event::time_span() const {
  if (empty())
    return 0;
  return (time_end() - time_start()) + uint16_t(1);
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
  return (earliest_end - latest_start) + uint16_t(1);
}

uint64_t Event::time_gap(const Cluster &other) const {
  if (empty() || other.empty()) {
    return 0; // \todo should this happen?
  }
  auto latest_start = std::max(other.time_start(), time_start());
  auto earliest_end = std::min(other.time_end(), time_end());

  if (latest_start <= earliest_end) {
    XTRACE(EVENT, DEB, "no time gap");
    return 0;
  }
  return (latest_start - earliest_end);
}


std::string Event::debug(bool verbose) const {
  auto ret = fmt::format("Event planes({}{},{}{}):",
                     plane1_, (cluster1.empty() ? "" : "*"),
                     plane2_, (cluster2.empty() ? "" : "*"));
  if (!cluster1.empty())
    ret += "\n  " + cluster1.debug(verbose);
  if (!cluster2.empty())
    ret += "\n  " + cluster2.debug(verbose);

  return ret;
}

std::string Event::visualize(uint8_t downsample_time,
                             uint8_t downsample_coords) const {
  auto ret = fmt::format("Event planes({},{}):",
                         plane1_, plane2_);
  if (!cluster1.empty())
    ret += "\n  " + cluster1.visualize(downsample_time, downsample_coords);
  if (!cluster2.empty())
    ret += "\n  " + cluster2.visualize(downsample_time, downsample_coords);
  return ret;
}

