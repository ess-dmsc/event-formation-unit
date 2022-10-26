// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief NMX detector
///
//===----------------------------------------------------------------------===//

#include <common/detector/Detector.h>

#include "NMXBase.h"

class NMX : public Nmx::NMXBase {
public:
  explicit NMX(BaseSettings Settings)
      : Nmx::NMXBase(std::move(Settings)) {}
};

DetectorFactory<NMX> Factory;
