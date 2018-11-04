/** Copyright (C) 2016, 2017 European Spallation Source ERIC */
//===----------------------------------------------------------------------===//
///
/// \file Event.cpp
/// \brief Event class implementation
///
//===----------------------------------------------------------------------===//

#include <common/clustering/Event.h>
#include <fmt/format.h>
#include <cmath>

Event::Event(uint8_t plane1, uint8_t plane2)
    : plane1_(plane1), plane2_(plane2) {}

void Event::insert_hit(const Hit &e) {
  if (e.plane == plane1_) {
    c1.insert_hit(e);
  } else if (e.plane == plane2_) {
    c2.insert_hit(e);
  }
}

void Event::merge(Cluster &cluster) {
  if (cluster.plane() == plane1_) {
    c1.merge(cluster);
  } else if (cluster.plane() == plane2_) {
    c2.merge(cluster);
  }
}

bool Event::empty() const {
  return c1.empty() && c2.empty();
}

uint64_t Event::time_end() const {
  if (c1.empty())
    return c2.time_end();
  if (c2.empty())
    return c1.time_end();
  return std::max(c1.time_end(), c2.time_end());
}

uint64_t Event::time_start() const {
  if (c1.empty())
    return c2.time_start();
  if (c2.empty())
    return c1.time_start();
  return std::min(c1.time_start(), c2.time_start());
}

uint64_t Event::time_span() const {
  if (empty())
    return 0;
  return (time_end() - time_start()) + uint16_t(1);
}

uint64_t Event::time_overlap(const Cluster &other) const {
  if (empty() || other.empty())
    return 0;
  auto latest_start = std::max(other.time_start(), time_start());
  auto earliest_end = std::min(other.time_end(), time_end());
  if (latest_start > earliest_end) {
    return 0;
  }
  return (earliest_end - latest_start) + uint16_t(1);
}

std::string Event::debug(bool verbose) const {
  return fmt::format("Event planes({},{}):\n  {}\n  {}\n",
                     plane1_, plane2_,
                     c1.debug(verbose), c2.debug(verbose));
}
