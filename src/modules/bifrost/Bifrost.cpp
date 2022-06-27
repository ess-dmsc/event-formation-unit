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

static Bifrost::BifrostSettings LocalBifrostSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
  // parser.add_option("-f, --file", LocalBifrostSettings.ConfigFile,
  //                   "Bifrost specific configuration (json) file")->group("BIFROST");
  // parser.add_option("--calibration", LocalLokiSettings.CalibFile,
  //                   "LoKI specific calibration (json) file")->group("LOKI");
  // parser.add_option("--dumptofile", LocalLokiSettings.FilePrefix,
  //                   "dump to specified file")->group("LOKI");
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class BIFROST : public Bifrost::BifrostBase {
public:
  explicit BIFROST(BaseSettings Settings)
      : Bifrost::BifrostBase(std::move(Settings), LocalBifrostSettings) {}
};

DetectorFactory<BIFROST> Factory;
