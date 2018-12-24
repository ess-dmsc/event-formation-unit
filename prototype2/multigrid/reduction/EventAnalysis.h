/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */

#pragma once

#include <common/clustering/Event.h>
#include <multigrid/geometry/SequoiaGeometry.h>
#include <limits>

namespace Multigrid
{

struct NeutronPosition {
  double x{std::numeric_limits<double>::quiet_NaN()};
  double y{std::numeric_limits<double>::quiet_NaN()};
  double z{std::numeric_limits<double>::quiet_NaN()};

  uint64_t time {0};
  bool good {false};

  std::string debug() const;
};

class mgAnalyzer {
 public:
  mgAnalyzer(bool weighted);

  NeutronPosition analyze(Event&) const;

  SequoiaGeometry mappings;

 private:
  bool weighted_{true};
};

}
