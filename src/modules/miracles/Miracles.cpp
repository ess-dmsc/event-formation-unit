// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Miracles detector base class - Miracles specific settings
///
//===----------------------------------------------------------------------===//

#include "MiraclesBase.h"
#include <common/detector/Detector.h>

static Miracles::MiraclesSettings LocalMiraclesSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
  parser
      .add_option("-f, --file", LocalMiraclesSettings.ConfigFile,
                  "Miracles specific configuration (json) file")
      ->group("MIRACLES");
  // parser.add_option("--calibration", LocalMiraclesSettings.CalibFile,
  //                   "Miracles specific calibration (json)
  //                   file")->group("MIRACLES");
  parser
      .add_option("--dumptofile", LocalMiraclesSettings.FilePrefix,
                  "dump to specified file")
      ->group("MIRACLES");
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class MIRACLES : public Miracles::MiraclesBase {
public:
  explicit MIRACLES(BaseSettings Settings)
      : Miracles::MiraclesBase(std::move(Settings), LocalMiraclesSettings) {}
};

DetectorFactory<MIRACLES> Factory;
