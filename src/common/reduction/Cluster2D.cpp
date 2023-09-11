// Copyright (C) 2023 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file Cluster2D.cpp
/// \brief Cluster2D class implementation
///
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <cmath>
#include <common/reduction/Cluster2D.h>
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

void Cluster2D::insert(const Hit2D &e) {

  DebugSplitOptimizer();

  if (hits.empty()) {
    time_start_ = time_end_ = e.time;
    x_coord_start_ = x_coord_end_ = x_coord_earliest_ = x_coord_latest_ =
        e.x_coordinate;
    y_coord_start_ = y_coord_end_ = y_coord_earliest_ = y_coord_latest_ =
        e.y_coordinate;
    utpc_idx_min_ = 0;
    utpc_idx_max_ = 0;
  }

  DebugSplitOptimizer();

  DebugSplitOptimizer();

  hits.push_back(e);

  DebugSplitOptimizer();

  /// \todo can we avoid converting to doubles and stay in uint64?
  double weight64 = static_cast<double>(e.weight);
  double x_coordinate64 = static_cast<double>(e.x_coordinate);
  double y_coordinate64 = static_cast<double>(e.y_coordinate);
  double time64 = static_cast<double>(e.time);

  DebugSplitOptimizer();

  double We = weight64;
  double We2 = weight64 * weight64;
  double WeXCo = weight64 * x_coordinate64;
  double We2XCo = weight64 * weight64 * x_coordinate64;
  double WeYCo = weight64 * y_coordinate64;
  double We2YCo = weight64 * weight64 * y_coordinate64;
  double WeTi = weight64 * time64;
  double We2Ti = weight64 * weight64 * time64;

  DebugSplitOptimizer();

  weight_sum_ += We;
  x_coord_mass_ += WeXCo;
  y_coord_mass_ += WeYCo;
  time_mass_ += WeTi;
  weight2_sum_ += We2;
  x_coord_mass2_ += We2XCo;
  y_coord_mass2_ += We2YCo;
  time_mass2_ += We2Ti;

  DebugSplitOptimizer();

  if (e.time < time_start_) {
    time_start_ = e.time;
    x_coord_earliest_ = e.x_coordinate;
    y_coord_earliest_ = e.y_coordinate;
  }

  DebugSplitOptimizer();

  // more than one hit with identical largest time in cluster
  if (e.time == time_end_) {
    utpc_idx_max_ = static_cast<int>(hits.size() - 1);
  } else if (e.time > time_end_) {
    utpc_idx_min_ = static_cast<int>(hits.size() - 1);
    utpc_idx_max_ = utpc_idx_min_;
    time_end_ = e.time;
    x_coord_latest_ = e.x_coordinate;
    y_coord_latest_ = e.y_coordinate;
  }

  DebugSplitOptimizer();

  x_coord_start_ = std::min(x_coord_start_, e.x_coordinate);
  x_coord_end_ = std::max(x_coord_end_, e.x_coordinate);
  y_coord_start_ = std::min(y_coord_start_, e.y_coordinate);
  y_coord_end_ = std::max(y_coord_end_, e.y_coordinate);
}

void Cluster2D::merge(Cluster2D &other) {
  if (other.hits.empty()) {
    return;
  }

  if (hits.empty()) {
    *this = std::move(other);
    return;
  }

  hits.reserve(std::max(hits.size() + other.hits.size(),
                        size_t(8))); // preallocate memory
  hits.insert(hits.end(), other.hits.begin(), other.hits.end());

  weight_sum_ += other.weight_sum_;
  weight2_sum_ += other.weight2_sum_;
  x_coord_mass_ += other.x_coord_mass_;
  x_coord_mass2_ += other.x_coord_mass2_;
  y_coord_mass_ += other.y_coord_mass_;
  y_coord_mass2_ += other.y_coord_mass2_;
  time_mass_ += other.time_mass_;
  time_mass2_ += other.time_mass2_;
  time_start_ = std::min(time_start_, other.time_start_);
  time_end_ = std::max(time_end_, other.time_end_);
  x_coord_start_ = std::min(x_coord_start_, other.x_coord_start_);
  x_coord_end_ = std::max(x_coord_end_, other.x_coord_end_);
  y_coord_start_ = std::min(y_coord_start_, other.y_coord_start_);
  y_coord_end_ = std::max(y_coord_end_, other.y_coord_end_);
  other.clear();
}

void Cluster2D::clear() {
  hits.clear();
  weight_sum_ = 0.0;
  weight2_sum_ = 0.0;
  x_coord_mass_ = 0.0;
  y_coord_mass_ = 0.0;
  time_mass_ = 0.0;
  x_coord_mass2_ = 0.0;
  y_coord_mass2_ = 0.0;
  time_mass2_ = 0.0;
}

bool Cluster2D::empty() const { return hits.empty(); }

bool Cluster2D::valid() const { return !hits.empty(); }

bool Cluster2D::hasGap(uint8_t MaxAllowedGap) const {
  return hits.size() + MaxAllowedGap < xCoordSpan();
}

size_t Cluster2D::hitCount() const { return hits.size(); }

uint16_t Cluster2D::xCoordStart() const { return x_coord_start_; }

uint16_t Cluster2D::xCoordEnd() const { return x_coord_end_; }

uint16_t Cluster2D::xCoordEarliest() const { return x_coord_earliest_; }

uint16_t Cluster2D::xCoordLatest() const { return x_coord_latest_; }

uint16_t Cluster2D::yCoordStart() const { return y_coord_start_; }

uint16_t Cluster2D::yCoordEnd() const { return y_coord_end_; }

uint16_t Cluster2D::yCoordEarliest() const { return y_coord_earliest_; }

uint16_t Cluster2D::yCoordLatest() const { return y_coord_latest_; }

uint16_t Cluster2D::xCoordSpan() const {
  if (hits.empty()) {
    return 0;
  }
  return (x_coord_end_ - x_coord_start_) + 1ul;
}

uint16_t Cluster2D::yCoordSpan() const {
  if (hits.empty()) {
    return 0;
  }
  return (y_coord_end_ - y_coord_start_) + 1ul;
}

uint64_t Cluster2D::timeStart() const { return time_start_; }

uint64_t Cluster2D::timeEnd() const { return time_end_; }

uint64_t Cluster2D::timeSpan() const {
  if (hits.empty()) {
    return 0;
  }
  return (time_end_ - time_start_) + 1ul;
}

double Cluster2D::weightSum() const { return weight_sum_; }

double Cluster2D::xCoordMass() const { return x_coord_mass_; }

double Cluster2D::xCoordCenter() const { return x_coord_mass_ / weight_sum_; }

double Cluster2D::yCoordMass() const { return y_coord_mass_; }

double Cluster2D::yCoordCenter() const { return y_coord_mass_ / weight_sum_; }

double Cluster2D::timeMass() const { return time_mass_; }

double Cluster2D::timeCenter() const { return time_mass_ / weight_sum_; }

double Cluster2D::xCoordMass2() const { return x_coord_mass2_; }

double Cluster2D::xCoordCenter2() const {
  return x_coord_mass2_ / weight2_sum_;
}

double Cluster2D::timeMass2() const { return time_mass2_; }

double Cluster2D::timeCenter2() const { return time_mass2_ / weight2_sum_; }

uint64_t Cluster2D::timeOverlap(const Cluster2D &other) const {
  if (empty() || other.empty())
    return 0;
  auto latest_start = std::max(other.time_start_, time_start_);
  auto earliest_end = std::min(other.time_end_, time_end_);
  if (latest_start > earliest_end) {
    return 0;
  }
  return (earliest_end - latest_start) + 1u;
}

uint64_t Cluster2D::timeGap(const Cluster2D &other) const {
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

std::string Cluster2D::to_string(const std::string &prepend,
                                 bool verbose) const {
  std::stringstream ss;
  ss << fmt::format("time=({},{})={} xspace=({},{})={} yspace=({},{})={} "
                    "weight={} entries[{}]",
                    time_start_, time_end_, timeSpan(), x_coord_start_,
                    x_coord_end_, xCoordSpan(), y_coord_start_, y_coord_end_,
                    yCoordSpan(), weight_sum_, hits.size());
  if (verbose && !hits.empty()) {
    ss << "\n";
    for (const auto &h : hits) {
      ss << prepend << "  " << h.to_string() << "\n";
    }
  }
  return ss.str();
}