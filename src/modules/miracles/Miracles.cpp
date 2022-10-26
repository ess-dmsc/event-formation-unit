// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Miracles detector base class - Miracles specific settings
///
//===----------------------------------------------------------------------===//

#include "MiraclesBase.h"
#include <common/detector/Detector.h>

class MIRACLES : public Miracles::MiraclesBase {
public:
  explicit MIRACLES(BaseSettings Settings)
      : Miracles::MiraclesBase(std::move(Settings)) {}
};

DetectorFactory<MIRACLES> Factory;
