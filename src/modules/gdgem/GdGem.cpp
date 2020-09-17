// Copyright (C) 2018-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief GdGEM base class - holds gdgem command line settings
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
  parser.add_option("--dumptofile", LocalNMXSettings.FilePrefix,
                    "dump to specified file")
      ->group("NMX")->configurable(true);

  // \todo REMOVE eventually
  parser.add_option("--pmin", LocalNMXSettings.PMin,
                    "min x-coordinate for this partition")
      ->group("NMX")->configurable(true);
  parser.add_option("--pmax", LocalNMXSettings.PMax,
                  "max x-coordinate for this partition")
      ->group("NMX")->configurable(true);
  parser.add_option("--pwidth", LocalNMXSettings.PWidth,
                    "Number of channels in overlap region")
      ->group("NMX")->configurable(true);
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class GdGem : public GdGemBase {
public:
  explicit GdGem(BaseSettings Settings)
      : GdGemBase(std::move(Settings), LocalNMXSettings) {}
};

DetectorFactory<GdGem> Factory;
