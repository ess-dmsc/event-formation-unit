/** Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file **/
//===----------------------------------------------------------------------===//
///
/// \file AbstractAnalyzer.h
/// \brief AbstractAnalyzer class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/reduction/Event.h>
#include <common/reduction/ReducedEvent.h>

/// \class AbstractAnalyzer AbstractAnalyzer.h
/// \brief AbstractAnalyzer declares the interface for an event analyzer type
///        that can take multi-dimensional Events and return a ReducedEvent
///        summaries that provide the best estimated coordinates and time
///        for a detected particle. Different strategies are to be implemented
///        for various detector types, depending on the particular physics.

class AbstractAnalyzer {
 public:
  AbstractAnalyzer() = default;
  virtual ~AbstractAnalyzer() = default;

  /// Assuming that only a subset of hits are used in determining particle position
  /// this helps keep track of how many were actually selected for averaging.
  mutable size_t stats_used_hits{0};

  /// \brief analyzes cluster in both planes
  virtual ReducedEvent analyze(Event&) const = 0;

  /// \brief prints info for debug purposes
  virtual std::string debug(const std::string& prepend) const = 0;
};
