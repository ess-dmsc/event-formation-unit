/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief MGMesytec detector
///
//===----------------------------------------------------------------------===//

#include <multigrid/MGMesytecBase.h>
#include <common/Detector.h>

static struct MGMesytecSettings LocalMGMesytecSettings;

void SetCLIArguments(CLI::App & parser) {
  parser.add_option("-f,--file", LocalMGMesytecSettings.ConfigFile,
                    "NMX (gdgem) specific config file")->group("MGMesytec")->
                    required()->configurable(true);
  parser.add_flag("--monitor", LocalMGMesytecSettings.monitor,
                  "stream monitor data")->group("MGMesytec")->configurable(true)->default_val("true");
  parser.add_option("--dumptofile", LocalMGMesytecSettings.fileprefix,
                    "dump to specified file")->group("MGMesytec")->configurable(true);
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class MGMesytec : public MGMesytecBase {
public:
  explicit MGMesytec(BaseSettings Settings)
      : MGMesytecBase(std::move(Settings), LocalMGMesytecSettings) {}
};

DetectorFactory<MGMesytec> Factory;
