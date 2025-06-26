// Copyright (C) 2024 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial CBM readouts
//===----------------------------------------------------------------------===//

// GCOVR_EXCL_START

#include <generators/functiongenerators/LinearGenerator.h>
#include <modules/cbm/generators/ReadoutGenerator.h>
#include <modules/cbm/geometry/Parser.h>

int main(int argc, char *argv[]) {

  cbm::ReadoutGenerator CbmGen;
  uint8_t cbmReadoutDataSize = sizeof(cbm::Parser::CbmReadout);
  CbmGen.setReadoutDataSize(cbmReadoutDataSize);

  CbmGen.argParse(argc, argv);

  std::unique_ptr<FunctionGenerator> readoutTimeGenerator;
  if (CbmGen.cbmSettings.monitorType == cbm::CbmType::EVENT_0D) {
    readoutTimeGenerator =
        std::make_unique<DistributionGenerator>(CbmGen.Settings.Frequency);
  } else if (CbmGen.cbmSettings.monitorType == cbm::CbmType::IBM) {
    readoutTimeGenerator = std::make_unique<LinearGenerator>(
        CbmGen.Settings.Frequency, CbmGen.cbmSettings.NumReadouts);
  } else {
    throw std::runtime_error("Unsupported monitor type");
  }

  CbmGen.initialize(std::move(readoutTimeGenerator));

  CbmGen.transmitLoop();
  return 0;
}
// GCOVR_EXCL_STOP
