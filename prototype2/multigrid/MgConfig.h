/// Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <logical_geometry/ESSGeometry.h>
#include <multigrid/mgmesytec/MgSeqGeometry.h>

struct MgConfig {
  MgConfig() {}
  MgConfig(std::string jsonfile);

  bool spoof_high_time{false};

  MgSeqGeometry mappings;

  std::string reduction_strategy;

  uint32_t wireThresholdLo{0};     // accept all
  uint32_t wireThresholdHi{65535}; // accept all
  uint32_t gridThresholdLo{0};     // accept all
  uint32_t gridThresholdHi{65535}; // accept all

  // Event formation
  ESSGeometry geometry;

  std::string debug() const;
};
