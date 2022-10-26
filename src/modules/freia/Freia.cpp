// Copyright (C) 2021 - 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Freia detector
///
//===----------------------------------------------------------------------===//

#include "FreiaBase.h"
#include <common/detector/Detector.h>

class FREIA : public Freia::FreiaBase {
public:
  explicit FREIA(BaseSettings Settings)
      : Freia::FreiaBase(std::move(Settings)) {}
};

DetectorFactory<FREIA> Factory;
