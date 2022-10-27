// Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief pixel generator (delete?)
/// \todo delete?
///
//===----------------------------------------------------------------------===//

#include "PerfGenBase.h"
#include <common/detector/Detector.h>

class PERFGEN : public PerfGen::PerfGenBase {
public:
  explicit PERFGEN(BaseSettings Settings)
      : PerfGen::PerfGenBase(std::move(Settings)) {}
};

DetectorFactory<PERFGEN> Factory;
