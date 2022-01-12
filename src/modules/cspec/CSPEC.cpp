// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CSPEC detector
///
//===----------------------------------------------------------------------===//

#include "CSPECBase.h"
#include <common/detector/Detector.h>

static struct Cspec::CSPECSettings LocalCSPECSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
  parser.add_option("--dumptofile", LocalCSPECSettings.FilePrefix,
                    "dump to specified file")->group("CSPEC");

  parser.add_option("-f, --file", LocalCSPECSettings.ConfigFile,
                    "CSPEC specific configuration (json) file")
                    ->group("CSPEC");
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class CSPEC : public Cspec::CSPECBase {
public:
  explicit CSPEC(BaseSettings Settings)
      : Cspec::CSPECBase(std::move(Settings), LocalCSPECSettings) {}
};

DetectorFactory<CSPEC> Factory;
