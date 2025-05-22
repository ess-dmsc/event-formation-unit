// Copyright (C) 2024 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial CBM readouts
//===----------------------------------------------------------------------===//

// GCOVR_EXCL_START

#include <modules/cbm/generators/ReadoutGenerator.h>
#include <generators/functiongenerators/LinearGenerator.h>
#include <modules/cbm/geometry/Parser.h>

int main(int argc, char *argv[]) {

  cbm::ReadoutGenerator CbmGen;
  uint8_t cbmReadoutDataSize = sizeof(cbm::Parser::CbmReadout);
  CbmGen.setReadoutDataSize(cbmReadoutDataSize);

  CbmGen.argParse(argc, argv);

  std::shared_ptr<FunctionGenerator> distribution;
  if (CbmGen.cbmSettings.monitorType == cbm::CbmType::EVENT_0D) {
    distribution = DistributionGenerator::Factory(CbmGen.Settings.Frequency);
  } else if (CbmGen.cbmSettings.monitorType == cbm::CbmType::IBM) {
    distribution = LinearGenerator::Factory(CbmGen.Settings.Frequency);
  } else {
    throw std::runtime_error("Unsupported monitor type");
  }

  CbmGen.initialize(distribution);

  CbmGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
