/** Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file **/
///===--------------------------------------------------------------------===///
///
/// \file HitVector.cpp
/// \brief HitVector alias and convenience functions
///
///===--------------------------------------------------------------------===///

#include <common/reduction/HitVector.h>
#include <algorithm>

#define ASCII_grayscale94 " .`:,;'_^\"></-!~=)(|j?}{][ti+l7v1%yrfcJ32uIC$zwo96sngaT5qpkYVOL40&mG8*xhedbZUSAQPFDXWK#RNEHBM@"
#define ASCII_grayscale70 " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$"
#define ASCII_grayscale10 " .:-=+*#%@"

#if ENABLE_GREEDY_HIT_ALLOCATOR
const size_t Bytes_2GB = 2ULL * 1024 * 1024 * 1024;
char *GreedyHitStorage::MemBegin = (char*)malloc(Bytes_2GB);
char *GreedyHitStorage::MemEnd = MemBegin + Bytes_2GB;
#else
char *GreedyHitStorage::MemBegin = nullptr;
char *GreedyHitStorage::MemEnd = nullptr;
#endif

HitVectorStorage::AllocConfig::PoolType* HitVectorStorage::Pool =
    new HitVectorStorage::AllocConfig::PoolType();

// Note: We purposefully leak the storage, since the EFU doesn't guaranteed that
// all memory is freed in the proper order (or at all).
PoolAllocator<HitVectorStorage::AllocConfig>
    HitVectorStorage::Alloc(*HitVectorStorage::Pool);

std::size_t HitVectorStorage::MaxAllocCount = 0;

std::string to_string(const HitVector &vec, const std::string &prepend) {
  std::stringstream ss;
  for (const auto &h : vec) {
    ss << prepend << h.to_string() << "\n";
  }
  return ss.str();
}

std::string visualize(const HitVector &vec, const std::string &prepend,
                      uint16_t max_columns, size_t max_rows) {
  if (vec.empty())
    return {};

  /// Survey bounds
  uint64_t time_min{std::numeric_limits<uint64_t>::max()};
  uint64_t time_max{0};
  uint16_t coord_min{std::numeric_limits<uint16_t>::max()};
  uint16_t coord_max{0};
  for (const auto &hit : vec) {
    time_min = std::min(time_min, hit.time);
    time_max = std::max(time_max, hit.time);
    coord_min = std::min(coord_min, hit.coordinate);
    coord_max = std::max(coord_max, hit.coordinate);
  }

  /// Rescale bounds
  uint64_t time_span = time_max - time_min;
  uint16_t coord_span = coord_max - coord_min;

  uint64_t time_bins = time_span;
  if (max_rows)
    time_bins = std::min(time_span, (uint64_t)max_rows);

  uint16_t coord_bins = coord_span;
  if (max_columns)
    coord_bins = std::min(coord_span, max_columns);

  double time_rescale = static_cast<double>(time_bins) / static_cast<double>(time_span);
  double coord_rescale = static_cast<double>(coord_bins) / static_cast<double>(coord_span);

  /// Build matrix
  uint16_t weight_max{0};
  std::vector<std::vector<uint16_t>> matrix;
  matrix.resize(time_bins + 1, std::vector<uint16_t>(coord_bins + 1, 0));
  for (const auto &h : vec) {
    auto t = (h.time - time_min) * time_rescale;
    auto c = (h.coordinate - coord_min) * coord_rescale;
    matrix[t][c] += h.weight;
    weight_max = std::max(weight_max, matrix[t][c]);
  }

  //////////////////////////////////
  /// Render as ASCII "colormap" ///
  //////////////////////////////////

  std::stringstream ss;

  /// Top frame
  ss << "\u2554";
  for (auto i = 0; i < coord_bins + 1; ++i)
    ss << "\u2550";
  ss << "\u2557\n";

  std::string representation(ASCII_grayscale94);
  for (size_t i = 0; i < matrix.size(); ++i) {
    const auto &row = matrix[i];
    ss << prepend << "\u2551";
    for (const auto &element : row) {
      ss << representation[93 * element / weight_max];
    }
    ss << "\u2551 ";
    ss << static_cast<uint64_t>(time_min + i / time_rescale);
    ss << "\n";
  }

  /// Bottom frame
  size_t longest_label{0};
  std::vector<std::string> bottom_labels;
  ss << "\u255A";
  for (auto i = 0; i < coord_bins + 1; ++i) {
    ss << "\u2550";
    auto label = std::to_string(static_cast<uint16_t>(coord_min + i / coord_rescale));
    longest_label = std::max(longest_label, label.size());
    bottom_labels.push_back(label);
  }
  ss << "\u255D\n";

  for (size_t i = 0; i < longest_label; ++i) {
    ss << " ";
    for (auto j = 0; j < coord_bins + 1; ++j) {
      const auto &label = bottom_labels[j];
      if (i < label.size())
        ss << label[i];
      else
        ss << " ";
    }
    ss << " \n";
  }
  return ss.str();
}
