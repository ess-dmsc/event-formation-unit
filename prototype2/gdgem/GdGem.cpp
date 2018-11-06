/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief GdGem detector
///
//===----------------------------------------------------------------------===//

#include "GdGemBase.h"
#include <common/Detector.h>

static struct NMXSettings LocalNMXSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) &parser) {
  parser.add_option("-f,--file", LocalNMXSettings.ConfigFile,
                    "NMX (gdgem) specific config (json) file")
      ->group("NMX")->required();
  parser.add_option("--calibration", LocalNMXSettings.CalibrationFile,
                    "NMX (gdgem) specific calibration (json) file")
      ->group("NMX");
  parser.add_option("--dumptofile", LocalNMXSettings.fileprefix,
                    "dump to specified file")
      ->group("NMX")->configurable(true);
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class GdGem : public GdGemBase {
public:
  explicit GdGem(BaseSettings Settings)
      : GdGemBase(std::move(Settings), LocalNMXSettings) {}
};

DetectorFactory<GdGem> Factory;
