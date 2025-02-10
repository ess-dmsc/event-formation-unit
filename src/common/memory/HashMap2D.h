// Copyright (C) 2024 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Wrapper for the std::map to access data through a 2D indexing as key
//===----------------------------------------------------------------------===//

#pragma once

#include <map>
#include <memory>
#include <vector>

/// \class A class template representing a 2D (column, row) map of values.
///
/// This class template provides a map-like container for storing and accessing
/// values. It allows adding and retrieving values based on a given
/// Column and Row index.
///
/// \tparam T The type of the values to be stored.
template <typename T> class HashMap2D {
public:

  /// Constructor to initialize the number of columns.
  HashMap2D(const int &NumColumns) : NumColumns(NumColumns) {}

  /// Copy assignment operator.
  ///
  /// This operator assigns the contents of another HashMap2D object to this
  /// object.
  ///
  /// \param other The HashMap2D object to be copied.
  /// \return A reference to this object after the assignment.
  HashMap2D<T> &operator=(const HashMap2D<T> &other) {
    if (this != &other) {
      NumColumns = other.NumColumns;
      ValueMap = other.ValueMap;
    }
    return *this;
  }

  /// Adds a value to the map.
  ///
  /// This function adds a value to the map based on the given Column
  /// and Row index. The value is moved into the map, so the original
  /// object is no longer valid after this function is called.
  ///
  /// \param Col The Column index.
  /// \param Row The Row index.
  /// \param Value The value to be added (the pointer is dereferenced and
  /// moved).
  inline void add(const int &Col, const int &Row, std::unique_ptr<T> &Value) {
    int Index = Row * NumColumns + Col;
    ValueMap.emplace(Index, std::move(Value));
  }

  /// Retrieves a value from the map.
  ///
  /// This function retrieves a reference to a value's unique ptr from the
  /// map based on the given Column and Row index.
  ///
  /// \param Col The Column index.
  /// \param Row The Row index.
  /// \return A reference to the value.
  /// \throws std::out_of_range if the value does not exist in the map.
  inline T *get(const int &Col, const int &Row) const {
    int Index = Row * NumColumns + Col;
    return ValueMap.at(Index).get();
  }

  /// Checks if a value exists in the map.
  ///
  /// This function checks if a value exists in the map based on the given
  /// Column and Row index.
  ///
  /// \param Col The Column index.
  /// \param Row The Row index.
  /// \return True if the value exists, false otherwise.
  inline bool isValue(const int &Col, const int &Row) const {
    int Index = Row * NumColumns + Col;
    return ValueMap.count(Index) > 0;
  }

  /// Retrieves all the values in the map.
  ///
  /// This function returns a reference to the map containing all the
  /// values.
  ///
  /// \return A reference to the map of values.
  std::vector<T *> toValuesList() const {
    std::vector<T *> Values;
    for (auto &Value : ValueMap) {
      Values.push_back(Value.second.get());
    }
    return Values;
  }

  /// Checks if the map is empty.
  ///
  /// This function checks if the map is empty.
  ///
  /// \return True if the map is empty, false otherwise.
  bool isEmpty() const {
    return ValueMap.empty();
  }

private:
  int NumColumns;
  std::map<int, std::unique_ptr<T>> ValueMap;
};
