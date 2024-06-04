// Copyright (C) 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Wrapper for the std::map to access data thorugh a 2D indexing as key
//===----------------------------------------------------------------------===//

#pragma once

#include <algorithm>
#include <map>
#include <memory>

/// @class A class template representing a 2D (column, row) map of values.
///
/// This class template provides a map-like container for storing and accessing
/// values. It allows adding and retrieving values based on a given
/// Column and Row index.
///
/// @tparam T The type of the values to be stored.
template <typename T> class HashMap2D {
public:
  /// Default constructor.
  HashMap2D(int NumColumns) : NumColumns(NumColumns) {}

  /// Adds a value to the map.
  ///
  /// This function adds a value to the map based on the given Column
  /// and Row index. The value is moved into the map, so the original
  /// object is no longer valid after this function is called.
  ///
  /// @param Col The Column index.
  /// @param Row The Row index.
  /// @param Value The value to be added (the pointer is dereferenced and
  /// moved).
  inline void add(int Col, int Row, std::unique_ptr<T> &Value) {
    int Index = Row * NumColumns + Col;
    ValueMap[Index] = std::move(Value);
  }

  /// Retrieves a value from the map.
  ///
  /// This function retrieves a reference to a value's unique ptr from the
  /// map based on the given Column and Row index.
  ///
  /// @param Col The Column index.
  /// @param Row The Row index.
  /// @return A reference to the value.
  /// @throws std::out_of_range if the value does not exist in the map.
  inline const std::unique_ptr<T> &get(int Col, int Row) const {
    int Index = Row * NumColumns + Col;
    return ValueMap.at(Index);
  }

  /// Retrieves all the values in the map.
  ///
  /// This function returns a reference to the map containing all the
  /// values.
  ///
  /// @return A reference to the map of values.
  std::map<int, std::unique_ptr<T>> &getAllValues() { return ValueMap; }

private:
  const int NumColumns;
  std::map<int, std::unique_ptr<T>> ValueMap;
};