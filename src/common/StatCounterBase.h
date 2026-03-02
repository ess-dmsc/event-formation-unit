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

/// \brief Base class that automatically registers counter members
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
/// - Compile-time safety: Must call base constructor or compilation fails
/// - Consistent naming: Automatic prefix handling with dots
/// - Memory aligned: Use __attribute__((aligned(64))) for performance
class StatCounterBase {
protected:
  /// \brief Type alias for counter name-reference pairs
  using CounterMap = std::vector<std::pair<std::string_view, std::reference_wrapper<int64_t>>>;

  /// \brief Constructor that automatically registers all counters from the map
  /// \param Stats Reference to Statistics object for registration
  /// \param Counters Vector of {name, counter_reference} pairs to register
  /// \param Prefix Optional prefix for counter names (automatically adds dot separator)
  StatCounterBase(Statistics& Stats, const CounterMap &Counters, const std::string &Prefix = "") {
    for (const auto& [name, counterRef] : Counters) {
     std::string fullName = Prefix.empty() ? std::string(name) : Prefix + "." + std::string(name);
     Stats.create(fullName, counterRef.get());
    }
  }

  /// \brief Virtual destructor for proper inheritance
  virtual ~StatCounterBase() = default;
};
