// Copyright (C) 2021 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Freia detector
///
//===----------------------------------------------------------------------===//

#include "FreiaBase.h"
#include <common/detector/Detector.h>

static struct Freia::FreiaSettings LocalFreiaSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
  parser.add_option("--dumptofile", LocalFreiaSettings.FilePrefix,
                    "dump to specified file")->group("Freia");

  parser.add_option("-f, --file", LocalFreiaSettings.ConfigFile,
                    "Freia specific configuration (json) file")
                    ->group("Freia");
  parser.add_option("--calibration", LocalFreiaSettings.CalibFile,
                    "Freia specific calibration (json) file")
                    ->group("Freia");
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class FREIA : public Freia::FreiaBase {
public:
  explicit FREIA(BaseSettings Settings)
      : Freia::FreiaBase(std::move(Settings), LocalFreiaSettings) {}
};

DetectorFactory<FREIA> Factory;
