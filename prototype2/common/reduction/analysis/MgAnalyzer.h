/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */

#pragma once

#include <common/reduction/analysis/AbstractAnalyzer.h>
#include <multigrid/geometry/ModuleGeometry.h>

class MGAnalyzer : public AbstractAnalyzer {
public:
  void weighted(bool w);
  void set_geometry(const Multigrid::ModuleGeometry& geom);
  Multigrid::ModuleGeometry geometry() const;

  /// \brief analyzes event in both planes
  ReducedEvent analyze(Event&) const override;

  /// \brief analyzes cluster as wires
  void analyze_wires(Cluster& cluster, ReducedHit& x, ReducedHit& z) const;

  /// \brief analyzes cluster as grids
  ReducedHit analyze_grids(Cluster& cluster) const;

  std::string debug(const std::string& prepend) const override;

  mutable size_t stats_used_hits{0};

private:
  bool weighted_{true};

  // \todo use pre-generated look-up table
  Multigrid::ModuleGeometry geometry_;
};
