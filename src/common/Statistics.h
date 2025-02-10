// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class for registering stat counters and associating them
/// with names. All counters are int64_t
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <string>
#include <vector>

class StatTuple {
public:
  /// \brief holds a name, value pair defining a 'stat'
  StatTuple(const std::string &Name, const int64_t &Value)
      : StatName(Name), StatValue(Value){};
  std::string StatName;
  const int64_t &StatValue;
};

class Statistics {
public:
  /// \brief null constructor
  Statistics() = default;

  /// \brief destructor deletes stats list
  ~Statistics() = default;

  /// \brief creates a 'stat' entry with name and address for counter
  /// duplicates are not allowed.
  int create(const std::string &StatName, int64_t &Value);

  /// \brief returns the number of registered stats
  size_t size();

  /// \brief returns the name of stat based on index
  std::string &name(size_t Index);

  /// \brief return value of stat based on index
  int64_t value(size_t Index);

  /// \brief return value by name
  int64_t valueByName(const std::string &name);

  /// \brief create grafana metric prefix by concatenation of strings
  /// PointChar will be added to the end
  void setPrefix(const std::string &StatsPrefix, const std::string &StatsRegion);

private:
  std::string prefix{""};       ///< prepend to all stat names
  std::vector<StatTuple> stats; ///< holds all registered stats
  std::string nostat{""};       ///< used to return when stats are not available
  const char PointChar = '.';
};
