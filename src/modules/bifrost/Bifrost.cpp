// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Bifrost detector base class - Bifrost specific settings
///
//===----------------------------------------------------------------------===//

#include "BifrostBase.h"
#include <common/detector/Detector.h>

class BIFROST : public Bifrost::BifrostBase {
public:
  explicit BIFROST(BaseSettings Settings)
      : Bifrost::BifrostBase(std::move(Settings)) {}
};

DetectorFactory<BIFROST> Factory;
