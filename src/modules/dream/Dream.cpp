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

static Dream::DreamSettings LocalDreamSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
  parser.add_option("-f, --file", LocalDreamSettings.ConfigFile,
      "Dream specific configuration (json) file")->group("DREAM");
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class DREAM : public Dream::DreamBase {
public:
  explicit DREAM(BaseSettings Settings)
      : Dream::DreamBase(std::move(Settings), LocalDreamSettings) {}
};

DetectorFactory<DREAM> Factory;
