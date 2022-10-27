// Copyright (C) 2021 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief DREAM detector base class - DREAM specific settings
///
//===----------------------------------------------------------------------===//

#include "DreamBase.h"
#include <common/detector/Detector.h>

class DREAM : public Dream::DreamBase {
public:
  explicit DREAM(BaseSettings Settings)
      : Dream::DreamBase(std::move(Settings)) {}
};

DetectorFactory<DREAM> Factory;
