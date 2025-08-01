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

// Forward declaration of StatTuple class
class StatTuple {
public:
  /// \brief Constructor for StatTuple
  /// \param Name the name of the stat
  /// \param Value the address of the counter to associate with the stat
  /// \param Prefix the prefix to prepend to the stat name, if empty,
  /// DefaultPrefix is used which is empty by default
  StatTuple(const std::string &Name, const int64_t &Value,
            const std::string &Prefix = "")
      : StatName(Name), StatValue(Value), StatPrefix(Prefix){};
  std::string StatName;
  const int64_t &StatValue;
  std::string StatPrefix; ///< prepend to all stat names
};

class Statistics {
public:
  /// \brief default constructor initializes stats list
  Statistics() = default;

  /// \brief constructor initializes stats list and sets prefix
  /// \param StatsPrefix the prefix to prepend to all stat names
  /// \param StatsRegion the region to prepend to all stat names
  /// \note DefaultPrefix is empty by default, so if StatsPrefix is empty,
  /// it will not be used
  Statistics(const std::string &StatsPrefix, const std::string &StatsRegion);

  /// \brief destructor deletes stats list
  ~Statistics() = default;

  /// \brief creates a 'stat' entry with name and address for counter
  /// \param StatName the name of the stat to create
  /// \param Value the address of the counter to associate with the stat
  /// \param Prefix the prefix to prepend to the stat name, if empty,
  /// DefaultPrefix is used which is empty by default
  /// \return 0 on success, -1 on error (duplicate name/prefix pair or address)
  /// \note duplicates are not allowed, and values are initialized to 0 during
  /// creation
  int create(const std::string &StatName, int64_t &Value,
             const std::string &Prefix = "");

  /// \brief returns the number of registered stats
  /// \return the number of registered stats
  /// \note this is the size of the stats vector but it is 1-based, so the first
  /// stat is at index 1
  size_t size();

  /// \brief returns the name of stat based on index
  /// \param Index the index of the stat to return
  /// \return the name of the stat or an empty string if not found
  /// \note the index is 1-based, so the first stat is at index
  std::string &getStatName(size_t Index);

  /// \brief returns the prefix of stat based on index
  /// \param Index the index of the stat to return
  /// \return the prefix of the stat or an empty string if not found
  std::string &getStatPrefix(size_t Index);

  /// \brief return a full name with prefix
  /// \param Index the index of the stat to return
  /// \return the full name of the stat or an empty string if not found
  /// \note the full name is created by concatenating the prefix, name and stat
  /// name
  std::string getFullName(size_t Index);

  /// \brief return value of stat based on index
  /// \param Index the index of the stat to return
  /// \return the value of the stat or -1 if not found
  int64_t getValue(size_t Index);

  /// \brief return value of stat based on name by seraching through all stats
  /// \note if multiple stats with the same name exist, the first one is
  /// returned \param Name the name of the stat to search for \param Prefix the
  /// prefix to search for, if empty, DefaultPrefix is used which is empty by
  /// default \return the value of the stat or -1 if not found
  int64_t getValueByName(const std::string &Name,
                         const std::string &Prefix = "");

  /// \brief set the default prefix
  /// \param StatsPrefix the prefix to prepend to all stat names
  /// \param StatsRegion the region to prepend to all stat names
  /// \note DefaultPrefix is empty by default but setting prefix here will
  /// change it for all stats created after this call
  /// \note already created stats will not be affected by this change
  void setPrefix(const std::string &StatsPrefix,
                 const std::string &StatsRegion);

private:
  std::string DefaultPrefix{""}; ///< prepend to all stat names
  std::vector<StatTuple> stats;  ///< holds all registered stats
  std::string nostat{""}; ///< used to return when stats are not available
  const char PointChar = '.';
};
