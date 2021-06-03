// Copyright (C) 2019-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief LoKI detector base class - LoKI specific settings
///
//===----------------------------------------------------------------------===//

#include "LokiBase.h"
#include <common/Detector.h>

static Loki::LokiSettings LocalLokiSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
  parser.add_option("-f, --file", LocalLokiSettings.ConfigFile,
                    "LoKI specific configuration (json) file")->group("LOKI");
  parser.add_option("--calibration", LocalLokiSettings.CalibFile,
                    "LoKI specific calibration (json) file")->group("LOKI");
  parser.add_option("--dumptofile", LocalLokiSettings.FilePrefix,
                    "dump to specified file")->group("LOKI");
  parser.add_option("--min", LocalLokiSettings.MinStraw,
                    "low values for straws")->group("LOKI");
  parser.add_option("--max", LocalLokiSettings.MaxStraw,
                    "low values for straws")->group("LOKI");
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class LOKI : public Loki::LokiBase {
public:
  explicit LOKI(BaseSettings Settings)
      : Loki::LokiBase(std::move(Settings), LocalLokiSettings) {}
};

DetectorFactory<LOKI> Factory;
