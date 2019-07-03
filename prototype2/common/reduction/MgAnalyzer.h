/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */

#pragma once

#include <common/reduction/AbstractAnalyzer.h>
#include <multigrid/geometry/ModuleGeometry.h>

class MGAnalyzer : public AbstractAnalyzer {
public:
  ReducedEvent analyze(Event &) const override;
  std::string debug() const override;

  void weighted(bool w);
  bool weighted() const;

  mutable size_t stats_used_hits{0};

  // \todo parametrize planes

  // \todo make private?
  Multigrid::ModuleLogicalGeometry geometry_;

private:
  bool weighted_{true};
};
