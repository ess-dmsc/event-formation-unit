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
#include <common/Detector.h>

static struct PerfGen::PerfGenSettings LocalPerfGenSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class PERFGEN : public PerfGen::PerfGenBase {
public:
  explicit PERFGEN(BaseSettings Settings)
      : PerfGen::PerfGenBase(std::move(Settings), LocalPerfGenSettings) {}
};

DetectorFactory<PERFGEN> Factory;
