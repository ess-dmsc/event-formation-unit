/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */

#pragma once

#include <common/analysis/AbstractAnalyzer.h>
#include <multigrid/geometry/ModuleGeometry.h>

namespace Gem {

class MGAnalyzer : public AbstractAnalyzer {
public:
  MultiDimResult analyze(Event&) const override;
  std::string debug() const override;

  void weighted(bool w);
  bool weighted() const;

  mutable size_t stats_used_hits {0};

  // \todo make private?
  Multigrid::ModuleLogicalGeometry geometry_;

private:
  bool weighted_{true};

};

}
