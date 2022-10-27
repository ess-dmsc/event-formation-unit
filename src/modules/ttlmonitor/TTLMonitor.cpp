// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief TTLMonitor detector
///
//===----------------------------------------------------------------------===//

#include "TTLMonitorBase.h"
#include <common/detector/Detector.h>

class TTLMON : public TTLMonitor::TTLMonitorBase {
public:
  explicit TTLMON(BaseSettings Settings)
      : TTLMonitor::TTLMonitorBase(std::move(Settings)) {}
};

DetectorFactory<TTLMON> Factory;
