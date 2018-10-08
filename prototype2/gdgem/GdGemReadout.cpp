/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief ADC readout detector module.
 */

#include "GdGemBase.h"
#include <common/Detector.h>

static NMXSettings LocalNMXSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
  parser
      .add_option("-f,--file", LocalNMXSettings.ConfigFile,
                  "NMX (gdgem) specific config (json) file")
      ->group("NMX")
      ->required();
  parser
      .add_option("--calibration", LocalNMXSettings.CalibrationFile,
                  "NMX (gdgem) specific calibration (json) file")
      ->group("NMX");
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class GdGem : public GdGemBase {
public:
  explicit GdGem(BaseSettings Settings)
      : GdGemBase(std::move(Settings), LocalNMXSettings) {}
};

DetectorFactory<GdGem> Factory;

// DetectorModuleRegistration::Registrar<AdcReadout> Register("AdcReadout",
//                                                           CLIArguments);
