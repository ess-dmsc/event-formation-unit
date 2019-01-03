/// Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <multigrid/AbstractBuilder.h>
#include <multigrid/reduction/Reduction.h>
#include <multigrid/reduction/EventAnalysis.h>
#include <logical_geometry/ESSGeometry.h>
#include <memory>

namespace Multigrid {

struct Config {
  Config() {}
  Config(std::string jsonfile, std::string dump_path = "");

  std::shared_ptr<AbstractBuilder> builder;
  Reduction reduction;
  EventAnalyzer analyzer;

  // Event formation
  ESSGeometry geometry;

  std::string debug() const;
};

}
