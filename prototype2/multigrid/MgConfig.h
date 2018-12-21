/// Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <logical_geometry/ESSGeometry.h>
#include <multigrid/geometry/SequoiaGeometry.h>
#include <string>

namespace Multigrid {

struct Config {
  Config() {}
  Config(std::string jsonfile);

  bool spoof_high_time{false};

  SequoiaGeometry mappings;

  std::string reduction_strategy;

  // Event formation
  ESSGeometry geometry;

  std::string debug() const;
};

}
