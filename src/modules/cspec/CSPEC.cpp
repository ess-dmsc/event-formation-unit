// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CSPEC detector
///
//===----------------------------------------------------------------------===//

#include <common/detector/Detector.h>

#include "CSPECBase.h"

class CSPEC : public Cspec::CSPECBase {
public:
  explicit CSPEC(BaseSettings Settings)
      : Cspec::CSPECBase(std::move(Settings)) {}
};

DetectorFactory<CSPEC> Factory;
