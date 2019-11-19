/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Loki detector
///
//===----------------------------------------------------------------------===//

#include "LokiBase.h"
#include <common/Detector.h>

static Loki::LokiSettings LocalLokiSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
  parser.add_option("-f, --file", LocalLokiSettings.ConfigFile,
                    "LoKI specific configuration (json) file")
                    ->group("LOKI");
  parser.add_option("--dumptofile", LocalLokiSettings.FilePrefix,
                    "dump to specified file")->group("LOKI");

  parser.add_flag("--2D", LocalLokiSettings.DetectorImage2D,
                    "Generate Pixels for 2D detector (else 3D)")
                    ->group("LOKI");
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class LOKI : public Loki::LokiBase {
public:
  explicit LOKI(BaseSettings Settings)
      : Loki::LokiBase(std::move(Settings), LocalLokiSettings) {}
};

DetectorFactory<LOKI> Factory;
