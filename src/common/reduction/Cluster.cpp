// Copyright (C) 2016, 2017 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file Cluster.cpp
/// \brief Cluster class implementation
///
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <cmath>
#include <common/reduction/Cluster.h>
#include <fmt/format.h>

#define ASCII_grayscale94                                                      \
  " .`:,;'_^\"></"                                                             \
  "-!~=)(|j?}{][ti+l7v1%yrfcJ32uIC$zwo96sngaT5qpkYVOL40&mG8*xhedbZUSAQPFDXWK#" \
  "RNEHBM@"
#define ASCII_grayscale70                                                      \
  " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$"
#define ASCII_grayscale10 " .:-=+*#%@"

// Debug code to try to split the basic blocks, to see what the optimizer is
// doing on which sections of code. Code taken from Google Benchmark
// ::benchmark::ClobberMemory().
// Idea:
// https://cellperformance.beyond3d.com/articles/2006/04/a-practical-gcc-trick-to-use-during-optimization.html
#if 0
__attribute__((noinline)) void DebugSplitOptimizer() {
  asm volatile("" : : : "memory");
}
#else
#define DebugSplitOptimizer() ((void)0)
#endif

void Cluster::insert(const Hit &e) {

  DebugSplitOptimizer();

  if (hits.empty()) {
    plane_ = e.plane;
    time_start_ = time_end_ = e.time;
    coord_start_ = coord_end_ = coord_earliest_ = coord_latest_ = e.coordinate;
    utpc_idx_min_ = 0;
    utpc_idx_max_ = 0;
  }

  DebugSplitOptimizer();

  // If plane identities don't match, invalidate
  if (plane_ != e.plane) {
    plane_ = Hit::InvalidPlane;
    // For now it's decided to not discard everything in this case
    // clear();
  }

  DebugSplitOptimizer();

  hits.push_back(e);

  DebugSplitOptimizer();

  /// \todo can we avoid converting to doubles and stay in uint64?
  double weight64 = static_cast<double>(e.weight);
  double coordinate64 = static_cast<double>(e.coordinate);
  double time64 = static_cast<double>(e.time);

  DebugSplitOptimizer();

  double We = weight64;
  double We2 = weight64 * weight64;
  double WeCo = weight64 * coordinate64;
  double We2Co = weight64 * weight64 * coordinate64;
  double WeTi = weight64 * time64;
  double We2Ti = weight64 * weight64 * time64;

  DebugSplitOptimizer();

  weight_sum_ += We;
  coord_mass_ += WeCo;
  time_mass_ += WeTi;
  weight2_sum_ += We2;
  coord_mass2_ += We2Co;
  time_mass2_ += We2Ti;

  DebugSplitOptimizer();

  if (e.time < time_start_) {
    time_start_ = e.time;
    coord_earliest_ = e.coordinate;
  }

  DebugSplitOptimizer();

  // more than one hit with identical largest time in cluster
  if (e.time == time_end_) {
    utpc_idx_max_ = static_cast<int>(hits.size() - 1);
  } else if (e.time > time_end_) {
    utpc_idx_min_ = static_cast<int>(hits.size() - 1);
    utpc_idx_max_ = utpc_idx_min_;
    time_end_ = e.time;
    coord_latest_ = e.coordinate;
  }

  DebugSplitOptimizer();

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
    plane_ = Hit::InvalidPlane;
    // For now it's decided to not discard everything in this case
    // clear();
  }

  hits.reserve(std::max(hits.size() + other.hits.size(),
                        size_t(8))); // preallocate memory
  hits.insert(hits.end(), other.hits.begin(), other.hits.end());

  weight_sum_ += other.weight_sum_;
  weight2_sum_ += other.weight2_sum_;
  coord_mass_ += other.coord_mass_;
  coord_mass2_ += other.coord_mass2_;
  time_mass_ += other.time_mass_;
  time_mass2_ += other.time_mass2_;
  time_start_ = std::min(time_start_, other.time_start_);
  time_end_ = std::max(time_end_, other.time_end_);
  coord_start_ = std::min(coord_start_, other.coord_start_);
  coord_end_ = std::max(coord_end_, other.coord_end_);
  other.clear();
}

void Cluster::clear() {
  hits.clear();
  plane_ = Hit::InvalidPlane;
  weight_sum_ = 0.0;
  weight2_sum_ = 0.0;
  coord_mass_ = 0.0;
  time_mass_ = 0.0;
  coord_mass2_ = 0.0;
  time_mass2_ = 0.0;
}

bool Cluster::empty() const { return hits.empty(); }

bool Cluster::valid() const {
  return !hits.empty() && (plane_ != Hit::InvalidPlane);
}

bool Cluster::hasGap(uint8_t MaxAllowedGap) const {
  return hits.size() + MaxAllowedGap < coordSpan();
}

uint8_t Cluster::plane() const { return plane_; }

size_t Cluster::hitCount() const { return hits.size(); }

uint16_t Cluster::coordStart() const { return coord_start_; }

uint16_t Cluster::coordEnd() const { return coord_end_; }

uint16_t Cluster::coordEarliest() const { return coord_earliest_; }

uint16_t Cluster::coordLatest() const { return coord_latest_; }

uint16_t Cluster::coordSpan() const {
  if (hits.empty()) {
    return 0;
  }
  return (coord_end_ - coord_start_) + 1ul;
}

uint64_t Cluster::timeStart() const { return time_start_; }

uint64_t Cluster::timeEnd() const { return time_end_; }

uint64_t Cluster::timeSpan() const {
  if (hits.empty()) {
    return 0;
  }
  return (time_end_ - time_start_) + 1ul;
}

double Cluster::weightSum() const { return weight_sum_; }

double Cluster::coordMass() const { return coord_mass_; }

double Cluster::coordCenter() const { return coord_mass_ / weight_sum_; }

double Cluster::timeMass() const { return time_mass_; }

double Cluster::timeCenter() const { return time_mass_ / weight_sum_; }

double Cluster::coordMass2() const { return coord_mass2_; }

double Cluster::coordCenter2() const { return coord_mass2_ / weight2_sum_; }

double Cluster::timeMass2() const { return time_mass2_; }

double Cluster::timeCenter2() const { return time_mass2_ / weight2_sum_; }

double Cluster::coordUtpc(bool weighted) const {
  int utpc_idx;
  if (utpc_idx_min_ == utpc_idx_max_) {
    utpc_idx = utpc_idx_max_;
  } else {
    if (utpc_idx_min_ < static_cast<int>(hits.size() - 1) - utpc_idx_max_) {
      utpc_idx = utpc_idx_min_;
    } else if (utpc_idx_min_ >
               static_cast<int>(hits.size() - 1) - utpc_idx_max_) {
      utpc_idx = utpc_idx_max_;
    } else {
      if (hits[utpc_idx_min_].weight > hits[utpc_idx_max_].weight) {
        utpc_idx = utpc_idx_min_;
      } else {
        utpc_idx = utpc_idx_max_;
      }
    }
  }

  if (!weighted) {
    return hits[utpc_idx].coordinate;
  }
  // utpc with center-of-mass: channels c and weights w
  double c1 = 0, c2 = 0, c3 = 0, w1 = 0, w2 = 0, w3 = 0;

  // coordinate and weight of hit with largest time
  c2 = hits[utpc_idx].coordinate;
  w2 = hits[utpc_idx].weight;
  // left neighbour of coordinate with largest time
  if (utpc_idx > 0) {
    c1 = hits[utpc_idx - 1].coordinate;
    w1 = hits[utpc_idx - 1].weight;
  }
  // right neighbour of coordinate with largest time
  if (utpc_idx < static_cast<int>(hits.size() - 1)) {
    c3 = hits[utpc_idx + 1].coordinate;
    w3 = hits[utpc_idx + 1].weight;
  }
  double pos_utpc = (c1 * w1 * w1 + c2 * w2 * w2 + c3 * w3 * w3) /
                    (w1 * w1 + w2 * w2 + w3 * w3);
  return pos_utpc;
}

uint64_t Cluster::timeOverlap(const Cluster &other) const {
  if (empty() || other.empty())
    return 0;
  auto latest_start = std::max(other.time_start_, time_start_);
  auto earliest_end = std::min(other.time_end_, time_end_);
  if (latest_start > earliest_end) {
    return 0;
  }
  return (earliest_end - latest_start) + 1u;
}

uint64_t Cluster::timeGap(const Cluster &other) const {
  if (empty() || other.empty()) {
    /// In case of two empty clusters time gap ought to be undefined or "inf"
    /// Returning max value of the used type, but throwing an exception
    /// could also be an option
    return std::numeric_limits<uint64_t>::max();
  }
  auto latest_start = std::max(other.time_start_, time_start_);
  auto earliest_end = std::min(other.time_end_, time_end_);
  if (latest_start <= earliest_end) {
    return 0;
  }
  return (latest_start - earliest_end);
}

std::string Cluster::to_string(const std::string &prepend, bool verbose) const {
  std::stringstream ss;
  ss << fmt::format(
      "plane={} time=({},{})={} space=({},{})={} weight={} entries[{}]", plane_,
      time_start_, time_end_, timeSpan(), coord_start_, coord_end_, coordSpan(),
      weight_sum_, hits.size());
  if (verbose && !hits.empty()) {
    ss << "\n";
    for (const auto &h : hits) {
      ss << prepend << "  " << h.to_string() << "\n";
    }
  }
  return ss.str();
}

std::string Cluster::visualize(const std::string &prepend,
                               uint8_t downsample_time,
                               uint8_t downsample_coords) const {

  auto t_span = ((time_end_ - time_start_) >> downsample_time) + 1u;
  auto c_span = ((coord_end_ - coord_start_) >> downsample_coords) + 1u;

  std::vector<std::vector<uint16_t>> matrix;
  matrix.resize(t_span, std::vector<uint16_t>(c_span, 0));
  uint16_t max_weight = 0;
  for (const auto &h : hits) {
    auto t = (h.time - time_start_) >> downsample_time;
    auto c = (h.coordinate - coord_start_) >> downsample_coords;
    matrix[t][c] += h.weight;
    max_weight = std::max(max_weight, matrix[t][c]);
  }

  std::string representation(ASCII_grayscale94);
  std::stringstream ss;
  for (const auto &row : matrix) {
    for (const auto &element : row) {
      if (element)
        ss << prepend << representation[93 * element / max_weight];
      else
        ss << " ";
    }
    ss << "\n";
  }
  return ss.str();
}
