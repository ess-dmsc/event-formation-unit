/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief MBCaen detector
///
//===----------------------------------------------------------------------===//

#include "MBCaenBase.h"
#include <common/Detector.h>

static struct MBCAENSettings LocalMBCAENSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
  parser.add_option("--dumptofile", LocalMBCAENSettings.FilePrefix,
                    "dump to specified file")->group("MBCAEN");

  parser.add_option("-f, --file", LocalMBCAENSettings.ConfigFile,
                    "Multi-Blade specific calibration (json) file")
                    ->group("MBCAEN");
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class MBCAEN : public MBCAENBase {
public:
  explicit MBCAEN(BaseSettings Settings)
      : MBCAENBase(std::move(Settings), LocalMBCAENSettings) {}
};

DetectorFactory<MBCAEN> Factory;
