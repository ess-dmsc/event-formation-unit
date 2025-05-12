// Copyright (C) 2024 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial CBM readouts
//===----------------------------------------------------------------------===//

// GCOVR_EXCL_START

#include <modules/cbm/generators/ReadoutGenerator.h>
#include <modules/cbm/geometry/Parser.h>

int main(int argc, char *argv[]) {

  cbm::ReadoutGenerator CbmGen;
  uint8_t cbmReadoutDataSize = sizeof(cbm::Parser::CbmReadout);
  CbmGen.setReadoutDataSize(cbmReadoutDataSize);

  CbmGen.argParse(argc, argv);

  std::shared_ptr<DistributionGenerator> distribution;
  if (CbmGen.cbmSettings.monitorType == cbm::CbmType::TTL) {
    distribution = DistributionGenerator::Factory(CbmGen.Settings.Frequency);
  } else if (CbmGen.cbmSettings.monitorType == cbm::CbmType::IBM) {
    distribution = std::make_unique<DistributionGenerator>( 1000.0 / CbmGen.Settings.Frequency);
  } else {
    throw std::runtime_error("Unsupported monitor type");
  }

  CbmGen.main(distribution);

  CbmGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
