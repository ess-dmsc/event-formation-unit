/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/clustering/Cluster.h>
#include <fmt/format.h>
#include <cmath>
#include <set>
#include <algorithm>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

void Cluster::insert_hit(const Hit &e) {
  if (entries.empty()) {
    plane = e.plane;
    time_start = time_end = e.time;
    coord_start = coord_end = e.coordinate;
  }

  // If plane identities don't match, invalidate
  /// \todo this needs more testing
  if (plane != e.plane) {
    plane = -1;
  }

  entries.push_back(e);
  weight_sum += e.weight;
  coord_mass += e.weight * e.coordinate;
  time_mass += e.weight * e.time;
  time_start = std::min(time_start, e.time);
  time_end = std::max(time_end, e.time);
  coord_start = std::min(coord_start, e.coordinate);
  coord_end = std::max(coord_end, e.coordinate);
}

bool Cluster::empty() const
{
  return entries.empty();
}

uint64_t Cluster::time_span() const {
  return time_end - time_start;
}

uint16_t Cluster::coord_span() const {
  if (entries.empty()) {
    return 0;
  }
  return (coord_end - coord_start) + 1u;
}

double Cluster::coord_center() const {
  return coord_mass / weight_sum;
}

double Cluster::time_center() const {
  return time_mass / weight_sum;
}

void Cluster::merge(Cluster &other) {
  if (other.entries.empty()) {
    return;
  }

  if (entries.empty()) {
    *this = std::move(other);
    return;
  }

  if (other.plane != plane) {
    return;
  }

  entries.reserve( entries.size() + other.entries.size() ); // preallocate memory
  entries.insert( entries.end(), other.entries.begin(), other.entries.end() );

  weight_sum += other.weight_sum;
  coord_mass += other.coord_mass;
  time_mass += other.time_mass;
  time_start = std::min(time_start, other.time_start);
  time_end = std::max(time_end, other.time_end);
  coord_start = std::min(coord_start, other.coord_start);
  coord_end = std::max(coord_end, other.coord_end);
}

/// \todo Unit tests for this
double Cluster::time_overlap(const Cluster &other) const {
  auto latest_start = std::max(other.time_start, time_start);
  auto earliest_end = std::min(other.time_end, time_end);
  if (latest_start > earliest_end) {
    return 0;
  }
  return earliest_end - latest_start;
}

/// \todo Precision of comparisons
/// \todo Comments or helper methods
bool Cluster::time_touch(const Cluster &other) const {
  if ((other.time_start == other.time_end) &&
      (time_start < other.time_end) && (other.time_end < time_end)) {
    return true;
  }
  if ((time_start == time_end) &&
      (other.time_start < time_end) && (time_end < other.time_end)) {
    return true;
  }
  return ((time_start == other.time_end) ||
      (time_end == other.time_start));
}

std::string Cluster::debug() const {
  return fmt::format("plane={} time=({},{}) space=({},{}) weight={} entries[{}]",
                     plane, time_start, time_end, coord_start, coord_end, weight_sum,
                     entries.size());
  //  for (const auto& e : entries)
  //    ss << e.debug() << "\n";
}
