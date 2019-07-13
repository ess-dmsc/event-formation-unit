/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */

#pragma once

#include <common/clustering/Event.h>
#include <common/reduction/ReducedEvent.h>
#include <multigrid/geometry/DigitalGeometry.h>
#include <limits>

namespace Multigrid {

class EventAnalyzer {
public:
  ReducedEvent analyze(Event &);

  void weighted(bool w);
  bool weighted() const;

  ModuleGeometry mappings;
  size_t stats_used_hits {0};

private:
  bool weighted_{true};
};

}
