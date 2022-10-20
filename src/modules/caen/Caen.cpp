// Copyright (C) 2019-2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CAEN detector base class - CAEN specific settings
///
//===----------------------------------------------------------------------===//

#include "CaenBase.h"
#include <common/detector/Detector.h>

static Caen::CaenSettings LocalCaenSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
  parser
      .add_option("-f, --file", LocalCaenSettings.ConfigFile,
                  "CAEN specific configuration (json) file")
      ->group("CAEN");
  parser
      .add_option("--calibration", LocalCaenSettings.CalibFile,
                  "CAEN specific calibration (json) file")
      ->group("CAEN");
  parser
      .add_option("--dumptofile", LocalCaenSettings.FilePrefix,
                  "dump to specified file")
      ->group("CAEN");
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class CAEN : public Caen::CaenBase {
public:
  explicit CAEN(BaseSettings Settings)
      : Caen::CaenBase(std::move(Settings), LocalCaenSettings) {}
};

DetectorFactory<CAEN> Factory;
