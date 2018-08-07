/// Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <logical_geometry/ESSGeometry.h>
#include <multigrid/mgmesytec/SequoiaGeometry.h>

struct MgConfig {
  MgConfig() {}
  MgConfig(std::string jsonfile);

  bool spoof_high_time{false};

  MgSeqGeometry mappings;

  std::string reduction_strategy;

  // Event formation
  ESSGeometry geometry;

  std::string debug() const;
};
