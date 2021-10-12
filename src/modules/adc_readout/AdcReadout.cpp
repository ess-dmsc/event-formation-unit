/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief ADC readout detector module.
 */

#include "AdcReadoutBase.h"
#include "AdcSettings.h"
#include <common/detector/Detector.h>
#include <common/detector/DetectorModuleRegister.h>

namespace AdcReadout {
static AdcSettings LocalAdcSettings;

void CLIArguments(CLI::App &Parser) {
  setCLIArguments(Parser, LocalAdcSettings);
}

class AdcReadout : public AdcReadoutBase {
public:
  explicit AdcReadout(BaseSettings const &Settings)
      : AdcReadoutBase(Settings, LocalAdcSettings) {}
};

REGISTER_MODULE(AdcReadout, CLIArguments);
} // namespace AdcReadout
