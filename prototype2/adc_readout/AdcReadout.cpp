/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief ADC readout detector module.
 */

#include "AdcReadoutBase.h"
#include "AdcSettings.h"
#include <common/Detector.h>
#include <common/DetectorModuleRegister.h>

static AdcSettings LocalAdcSettings;

void CLIArguments(CLI::App &parser) {
  SetCLIArguments(parser, LocalAdcSettings);
}

PopulateCLIParser PopulateParser{CLIArguments};

class AdcReadout : public AdcReadoutBase {
public:
  explicit AdcReadout(BaseSettings Settings)
      : AdcReadoutBase(std::move(Settings), LocalAdcSettings) {}
};

DetectorFactory<AdcReadout> Factory;

DetectorModuleRegistration::Registrar<AdcReadout> Register("AdcReadout",
                                                           CLIArguments);
