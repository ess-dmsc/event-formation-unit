// Copyright (C) 2019-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief LoKI detector base class - LoKI specific settings
///
//===----------------------------------------------------------------------===//

#include "LokiBase.h"
#include <common/detector/Detector.h>

class LOKI : public Loki::LokiBase {
public:
  explicit LOKI(BaseSettings Settings)
      : Loki::LokiBase(std::move(Settings)) {}
};

DetectorFactory<LOKI> Factory;
