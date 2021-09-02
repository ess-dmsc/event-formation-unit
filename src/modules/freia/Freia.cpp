// Copyright (C) 2017-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Freia detector
///
//===----------------------------------------------------------------------===//

#include "FreiaBase.h"
#include <common/Detector.h>

static struct Freia::FreiaSettings LocalFreiaSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
  parser.add_option("--dumptofile", LocalFreiaSettings.FilePrefix,
                    "dump to specified file")->group("Freia");

  parser.add_option("-f, --file", LocalFreiaSettings.ConfigFile,
                    "Multi-Blade specific calibration (json) file")
                    ->group("Freia");

  parser.add_option("--h5filesplit", LocalFreiaSettings.H5SplitTime,
                    "Specify interval to split HDF5 files")
                    ->group("Freia");
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class FREIA : public Freia::FreiaBase {
public:
  explicit FREIA(BaseSettings Settings)
      : Freia::FreiaBase(std::move(Settings), LocalFreiaSettings) {}
};

DetectorFactory<FREIA> Factory;
