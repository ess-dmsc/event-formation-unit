/** Copyright (C) 2016, 2017 European Spallation Source ERIC */
//===----------------------------------------------------------------------===//
///
/// \file Cluster.cpp
/// \brief Cluster class implementation
///
//===----------------------------------------------------------------------===//

#include <common/clustering/Cluster.h>
#include <fmt/format.h>
#include <cmath>

void Cluster::insert(const Hit &e) {
  if (hits.empty()) {
    plane_ = e.plane;
    time_start_ = time_end_ = e.time;
    coord_start_ = coord_end_ = e.coordinate;
  }

  // If plane identities don't match, invalidate
  if (plane_ != e.plane) {
    plane_ = -1;
    // For now it's decided to not discard everything in this case
    // clear();
  }

  hits.push_back(e);
  weight_sum_ += e.weight;
  coord_mass_ += e.weight * e.coordinate;
  time_mass_ += e.weight * e.time;
  time_start_ = std::min(time_start_, e.time);
  time_end_ = std::max(time_end_, e.time);
  coord_start_ = std::min(coord_start_, e.coordinate);
  coord_end_ = std::max(coord_end_, e.coordinate);
}

void Cluster::merge(Cluster &other) {
  if (other.hits.empty()) {
    return;
  }

  if (hits.empty()) {
    *this = std::move(other);
    return;
  }

  // If plane identities don't match, invalidate
  if (other.plane_ != plane_) {
    plane_ = -1;
    // For now it's decided to not discard everything in this case
    // clear();
  }

  hits.reserve(hits.size() + other.hits.size()); // preallocate memory
  hits.insert(hits.end(), other.hits.begin(), other.hits.end());

  weight_sum_ += other.weight_sum_;
  coord_mass_ += other.coord_mass_;
  time_mass_ += other.time_mass_;
  time_start_ = std::min(time_start_, other.time_start_);
  time_end_ = std::max(time_end_, other.time_end_);
  coord_start_ = std::min(coord_start_, other.coord_start_);
  coord_end_ = std::max(coord_end_, other.coord_end_);
  other.clear();
}

void Cluster::clear() {
  hits.clear();
  plane_ = -1;
  weight_sum_ = 0.0;
  coord_mass_ = 0.0;
  time_mass_ = 0.0;
}

bool Cluster::empty() const {
  return hits.empty();
}

bool Cluster::valid() const {
  return !hits.empty() && (plane_ >= 0);
}

int16_t Cluster::plane() const {
  return plane_;
}

size_t Cluster::hit_count() const {
  return hits.size();
}

uint16_t Cluster::coord_start() const {
  return coord_start_;
}

uint16_t Cluster::coord_end() const {
  return coord_end_;
}

uint16_t Cluster::coord_span() const {
  if (hits.empty()) {
    return 0;
  }
  return (coord_end_ - coord_start_) + uint16_t(1);
}

uint64_t Cluster::time_start() const {
  return time_start_;
}

uint64_t Cluster::time_end() const {
  return time_end_;
}

uint64_t Cluster::time_span() const {
  if (hits.empty()) {
    return 0;
  }
  return (time_end_ - time_start_) + uint64_t(1);
}

double Cluster::weight_sum() const {
  return weight_sum_;
}

double Cluster::coord_mass() const {
  return coord_mass_;
}

double Cluster::coord_center() const {
  return coord_mass_ / weight_sum_;
}

double Cluster::time_mass() const {
  return time_mass_;
}

double Cluster::time_center() const {
  return time_mass_ / weight_sum_;
}

uint64_t Cluster::time_overlap(const Cluster &other) const {
  if (empty() || other.empty())
    return 0;
  auto latest_start = std::max(other.time_start_, time_start_);
  auto earliest_end = std::min(other.time_end_, time_end_);
  if (latest_start > earliest_end) {
    return 0;
  }
  return (earliest_end - latest_start) + uint16_t(1);
}

uint64_t Cluster::time_gap(const Cluster &other) const {
  if (empty() || other.empty())
    return 0; // \todo should this happen?
  auto latest_start = std::max(other.time_start_, time_start_);
  auto earliest_end = std::min(other.time_end_, time_end_);
  if (latest_start <= earliest_end) {
    return 0;
  }
  return (latest_start - earliest_end);
}

std::string Cluster::debug(bool verbose) const {
  if (!verbose) {
    return fmt::format("plane={} time=({},{}) space=({},{}) weight={} entries[{}]",
                       plane_, time_start_, time_end_, coord_start_, coord_end_, weight_sum_,
                       hits.size());
  }
  auto ret = debug(false);
  for (const auto &h : hits) {
    ret += "\n   " + h.debug();
  }
  return ret;
}
