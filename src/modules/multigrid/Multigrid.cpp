/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Multigrid detector
///
//===----------------------------------------------------------------------===//

#include <multigrid/MultigridBase.h>
#include <common/Detector.h>

static struct MultigridSettings LocalMultigridSettings;

void SetCLIArguments(CLI::App & parser) {
  parser.add_option("-f,--file", LocalMultigridSettings.ConfigFile,
                    "NMX (gdgem) specific config file")->group("Multigrid")->
                    required()->configurable(true);
  parser.add_flag("--monitor", LocalMultigridSettings.monitor,
                  "stream monitor data")->group("Multigrid")->configurable(true)->default_val("true");
  parser.add_option("--dumptofile", LocalMultigridSettings.FilePrefix,
                    "dump to specified file")->group("Multigrid")->configurable(true);
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class MultiGrid : public MultigridBase {
public:
  explicit MultiGrid(BaseSettings Settings)
      : MultigridBase(std::move(Settings), LocalMultigridSettings) {}
};

DetectorFactory<MultiGrid> Factory;
