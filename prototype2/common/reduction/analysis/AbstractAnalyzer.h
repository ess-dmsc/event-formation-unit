/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/reduction/Event.h>
#include <common/reduction/ReducedEvent.h>

class AbstractAnalyzer {
 public:
  AbstractAnalyzer() = default;
  virtual ~AbstractAnalyzer() = default;

  /// \brief analyzes cluster in both planes
  virtual ReducedEvent analyze(Event&) const = 0;

  virtual std::string debug(const std::string& prepend) const = 0;
};
