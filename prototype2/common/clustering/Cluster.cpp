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

#define ASCII_grayscale94 " .`:,;'_^\"></-!~=)(|j?}{][ti+l7v1%yrfcJ32uIC$zwo96sngaT5qpkYVOL40&mG8*xhedbZUSAQPFDXWK#RNEHBM@"
#define ASCII_grayscale70 " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$"
#define ASCII_grayscale10 " .:-=+*#%@"

void Cluster::insert(const Hit &e)
{
  if (hits.empty())
  {
    plane_ = e.plane;
    time_start_ = time_end_ = e.time;
    coord_start_ = coord_end_ = e.coordinate;
  }

  // If plane identities don't match, invalidate
  if (plane_ != e.plane)
  {
    plane_ = -1;
    // For now it's decided to not discard everything in this case
    // clear();
  }

  hits.push_back(e);
  weight_sum_ += e.weight;
  weight2_sum_ += e.weight * e.weight;
  coord_mass_ += e.weight * e.coordinate;
  coord_mass2_ += e.weight * e.weight * e.coordinate;
  time_mass_ += e.weight * e.time;
  time_mass2_ += e.weight * e.weight * e.time;
  time_start_ = std::min(time_start_, e.time);

  if (e.time > time_end_)
  {
    time_end_ = e.time;
    weight_utpc_sum_ = e.weight;
    cnt_utpc_ = 1;
    coord_utpc_ = e.coordinate;
    time_utpc_ = e.time;
    coord_mass_utpc_ = e.coordinate * e.weight;
    time_mass_utpc_ = e.time * e.weight;
    lspan_min_ = static_cast<int16_t>(e.coordinate);
    lspan_max_ = static_cast<int16_t>(e.coordinate);
  }
  else if (e.time == time_end_)
  {
    weight_utpc_sum_ += e.weight;
    cnt_utpc_++;
    coord_utpc_ += e.coordinate;
    time_utpc_ += e.time;
    coord_mass_utpc_ += e.coordinate * e.weight;
    time_mass_utpc_ += e.time * e.weight;
    lspan_min_ = std::min(lspan_min_, static_cast<int16_t>(e.coordinate));
    lspan_max_ = std::max(lspan_max_, static_cast<int16_t>(e.coordinate));
  }
  coord_start_ = std::min(coord_start_, e.coordinate);
  coord_end_ = std::max(coord_end_, e.coordinate);
}

void Cluster::merge(Cluster &other)
{
  if (other.hits.empty())
  {
    return;
  }

  if (hits.empty())
  {
    *this = std::move(other);
    return;
  }

  // If plane identities don't match, invalidate
  if (other.plane_ != plane_)
  {
    plane_ = -1;
    // For now it's decided to not discard everything in this case
    // clear();
  }

  hits.reserve(hits.size() + other.hits.size()); // preallocate memory
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

void Cluster::clear()
{
  hits.clear();
  plane_ = -1;
  weight_sum_ = 0.0;
  weight2_sum_ = 0.0;
  coord_mass_ = 0.0;
  time_mass_ = 0.0;
  coord_mass2_ = 0.0;
  time_mass2_ = 0.0;
}

bool Cluster::empty() const
{
  return hits.empty();
}

bool Cluster::valid() const
{
  return !hits.empty() && (plane_ >= 0);
}

int16_t Cluster::plane() const
{
  return plane_;
}

size_t Cluster::hit_count() const
{
  return hits.size();
}

uint16_t Cluster::coord_start() const
{
  return coord_start_;
}

uint16_t Cluster::coord_end() const
{
  return coord_end_;
}

uint16_t Cluster::coord_span() const
{
  if (hits.empty())
  {
    return 0;
  }
  return (coord_end_ - coord_start_) + uint16_t(1);
}

uint64_t Cluster::time_start() const
{
  return time_start_;
}

uint64_t Cluster::time_end() const
{
  return time_end_;
}

uint64_t Cluster::time_span() const
{
  if (hits.empty())
  {
    return 0;
  }
  return (time_end_ - time_start_) + uint64_t(1);
}

double Cluster::weight_sum() const
{
  return weight_sum_;
}

double Cluster::weight2_sum() const
{
  return weight2_sum_;
}

double Cluster::coord_mass() const
{
  return coord_mass_;
}

double Cluster::coord_center() const
{
  return coord_mass_ / weight_sum_;
}

double Cluster::time_mass() const
{
  return time_mass_;
}

double Cluster::time_center() const
{
  return time_mass_ / weight_sum_;
}

double Cluster::coord_mass2() const
{
  return coord_mass2_;
}

double Cluster::coord_center2() const
{
  return coord_mass2_ / weight2_sum_;
}

double Cluster::time_mass2() const
{
  return time_mass2_;
}

double Cluster::time_center2() const
{
  return time_mass2_ / weight2_sum_;
}

double Cluster::coord_utpc(bool weighted) const
{
  if (!weighted)
  {
    return coord_utpc_ / cnt_utpc_;
  }
  return coord_mass_utpc_ / weight_utpc_sum_;
}

double Cluster::time_utpc(bool weighted) const
{
  if (!weighted)
  {
    return time_utpc_ / cnt_utpc_;
  }
  return time_mass_utpc_ / weight_utpc_sum_;
}

int16_t Cluster::lspan_min()
{
  return lspan_min_;
}

int16_t Cluster::lspan_max()
{
  return lspan_max_;
}

uint64_t Cluster::time_overlap(const Cluster &other) const
{
  if (empty() || other.empty())
    return 0;
  auto latest_start = std::max(other.time_start_, time_start_);
  auto earliest_end = std::min(other.time_end_, time_end_);
  if (latest_start > earliest_end)
  {
    return 0;
  }
  return (earliest_end - latest_start) + uint16_t(1);
}

uint64_t Cluster::time_gap(const Cluster &other) const
{
  if (empty() || other.empty())
    return 0; // \todo should this happen?
  auto latest_start = std::max(other.time_start_, time_start_);
  auto earliest_end = std::min(other.time_end_, time_end_);
  if (latest_start <= earliest_end)
  {
    return 0;
  }
  return (latest_start - earliest_end);
}

std::string Cluster::debug(bool verbose) const
{
  if (!verbose)
  {
    return fmt::format("plane={} time=({},{})={} space=({},{})={} weight={} entries[{}]",
                       plane_,
                       time_start_, time_end_, time_span(),
                       coord_start_, coord_end_, coord_span(),
                       weight_sum_,
                       hits.size());
  }
  auto ret = debug(false);
  for (const auto &h : hits)
  {
    ret += "\n   " + h.debug();
  }
  return ret;
}

std::string Cluster::visualize(uint8_t downsample_time,
                               uint8_t downsample_coords) const
{

  auto t_span = ((time_end_ - time_start_) >> downsample_time) + 1;
  auto c_span = ((coord_end_ - coord_start_) >> downsample_coords) + 1;

  std::vector<std::vector<uint16_t>> matrix;
  matrix.resize(t_span, std::vector<uint16_t>(c_span, 0));
  uint16_t max_weight = 0;
  for (const auto &h : hits)
  {
    auto t = (h.time - time_start_) >> downsample_time;
    auto c = (h.coordinate - coord_start_) >> downsample_coords;
    matrix[t][c] += h.weight;
    max_weight = std::max(max_weight, matrix[t][c]);
  }

  std::string representation(ASCII_grayscale94);
  auto ret = debug(false) + "\n";
  for (const auto &row : matrix)
  {
    for (const auto &element : row)
    {
      if (element)
        ret += representation[93 * element / max_weight];
      else
        ret += " ";
    }
    ret += "\n";
  }
  return ret;
}
