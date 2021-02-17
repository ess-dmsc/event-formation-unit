// Copyright (C) 2021 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief DREAM detector base class - DREAM specific settings
///
//===----------------------------------------------------------------------===//

#include "DreamBase.h"
#include <common/Detector.h>

static Jalousie::DreamSettings LocalDreamSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class DREAM : public Jalousie::DreamBase {
public:
  explicit DREAM(BaseSettings Settings)
      : Jalousie::DreamBase(std::move(Settings), LocalDreamSettings) {}
};

DetectorFactory<DREAM> Factory;
