// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Base class for automatic statistics counter registration
///
/// This class provides a simple pattern for automatic registration of
/// counter variables to the Statistics system. Derived classes must
/// call the base constructor with a map of counter names and references,
/// ensuring compile-time enforcement of proper registration.
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Statistics.h>
#include <functional>
#include <string>
#include <vector>

/// \brief Base class that automatically registers counters
/// to the Statistics system during construction.
///
/// Usage:
///   struct MyComponentStats : public StatCounterBase {
///     int64_t ErrorCount{0};
///     int64_t ProcessedItems{0};
///     
///     MyComponentStats(Statistics& Stats) 
///       : StatCounterBase(Stats, {
///           {"errors", ErrorCount},
///           {"processed", ProcessedItems}
///         }, "mycomponent") {}
///   } __attribute__((aligned(64)));
///
/// Benefits:
/// - Automatic registration: No manual Stats.create() calls needed
/// - Protected constructor: Prevents direct instantiation, inheritance only
/// - Consistent naming: Automatic prefix handling with dots
/// - Performance tip: Use __attribute__((aligned(64))) on derived structs
class StatCounterBase {
protected:
  /// \brief Type alias for counter name-reference pairs
  using CounterMap = std::vector<std::pair<std::string, std::reference_wrapper<int64_t>>>;

  /// \brief Constructor that automatically registers all counters from the map
  /// \param Stats Reference to Statistics object for registration
  /// \param Counters Vector of {name, counter_reference} pairs to register
  /// \param Prefix Optional prefix for counter names (automatically adds dot separator)
  StatCounterBase(Statistics& Stats, const CounterMap& Counters, const std::string& Prefix = "") {
    for (const auto& [name, counterRef] : Counters) {
      std::string fullName = Prefix.empty() ? name : Prefix + "." + name;
      Stats.create(fullName, counterRef.get());
    }
  }

  /// \brief Virtual destructor to ensure proper cleanup in derived classes
  virtual ~StatCounterBase() = default;
};
