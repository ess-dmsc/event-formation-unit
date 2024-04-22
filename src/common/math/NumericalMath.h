// Copyright (C) 2024 European Spallation Source, ERIC. see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of BinAggregation methods
//===----------------------------------------------------------------------===//

#pragma once

#include <functional>
#include <numeric>
#include <stdexcept>

namespace essmath {

template <typename T> using VectorAggregationFunc = std::function<T(std::vector<T>)>;

// Implementation of average aggregation function
template <typename T> T average(const std::vector<T> &data) {
  if (data.empty()) {
    throw std::runtime_error("Cannot calculate average of an empty vector");
  }

  T sum = std::accumulate(data.begin(), data.end(), T(0));
  return sum / data.size();
}

// Implementation of sum aggregation function
template <typename T> constexpr T sum(const std::vector<T> &data) {
  return std::accumulate(data.begin(), data.end(), T(0));
}

template <class T> VectorAggregationFunc<T> SUM_AGG_FUNC = sum<T>;
template <class T> VectorAggregationFunc<T> AVERAGE_AGG_FUNC = average<T>;

} // namespace serializer