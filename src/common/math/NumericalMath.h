// Copyright (C) 2024 European Spallation Source, ERIC. see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of BinAggregation methods
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>
#include <functional>
#include <numeric>
#include <stdexcept>

///
/// \namespace essmath
/// \brief Contains mathematical functions and utilities for numerical
/// operations.
///
namespace essmath {

///
/// \typedef VectorAggregationFunc
/// \brief A type alias for a function that aggregates a vector of values.
/// \tparam T The type of the values to be aggregated.
///
template <typename T>
using VectorAggregationFunc = std::function<T(std::pair<T, uint32_t>)>;

///
/// \fn average
/// \brief Calculates the average of a given pair of total value and count.
/// \tparam T The type of the value.
/// \param data A pair containing the total value and the count.
/// \return The average value.
/// \throws std::runtime_error if the input pair is empty.
///
template <typename T> T average(const std::pair<T, uint32_t> &data) {
  if (data.second == 0) {
    return data.first;
    throw std::runtime_error("Cannot calculate average of an empty value pair");
  }

  return data.first / data.second;
}

///
/// \fn sum
/// \brief Returns the sum of a given pair of total value and count.
/// \tparam T The type of the value.
/// \param data A pair containing the total value and the count.
/// \return The total value.
///
template <typename T> constexpr T sum(const std::pair<T, uint32_t> &data) {
  return data.first;
}

///
/// \var SUM_AGG_FUNC
/// \brief A function object for summing values.
/// \tparam T The type of the value.
///
template <class T> VectorAggregationFunc<T> SUM_AGG_FUNC = sum<T>;

///
/// \var AVERAGE_AGG_FUNC
/// \brief A function object for calculating the average of values.
/// \tparam T The type of the value.
///
template <class T> VectorAggregationFunc<T> AVERAGE_AGG_FUNC = average<T>;

} // namespace essmath