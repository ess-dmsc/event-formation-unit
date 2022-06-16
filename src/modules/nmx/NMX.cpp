// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief NMX detector
///
//===----------------------------------------------------------------------===//

#include <common/detector/Detector.h>

#include "NMXBase.h"

static struct Nmx::NMXSettings LocalNMXSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
  parser
      .add_option("--dumptofile", LocalNMXSettings.FilePrefix,
                  "dump to specified file")
      ->group("NMX");

  parser
      .add_option("-f, --file", LocalNMXSettings.ConfigFile,
                  "NMX specific configuration (json) file")
      ->group("NMX");
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class NMX : public Nmx::NMXBase {
public:
  explicit NMX(BaseSettings Settings)
      : Nmx::NMXBase(std::move(Settings), LocalNMXSettings) {}
};

DetectorFactory<NMX> Factory;
