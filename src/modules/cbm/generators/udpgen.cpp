// Copyright (C) 2024-2026 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial CBM readouts
//===----------------------------------------------------------------------===//

// GCOVR_EXCL_START

#include <modules/cbm/generators/DataGeneratorFactory.h>
#include <modules/cbm/generators/TimeGeneratorFactory.h>
#include <modules/cbm/generators/ReadoutGenerator.h>
#include <modules/cbm/readout/Parser.h>

using namespace cbm;

int main(int argc, char *argv[]) {

  ReadoutGenerator CbmGen;
  uint8_t cbmReadoutDataSize = sizeof(Parser::CbmReadout);
  CbmGen.setReadoutDataSize(cbmReadoutDataSize);

  CbmGen.argParse(argc, argv);

  CbmGen.initialize(TimeGeneratorFactory::createTimeGenerator(
      CbmGen.cbmSettings.monitorType, CbmGen.Settings.Frequency,
      CbmGen.cbmSettings.NumReadouts));

  CbmGen.transmitLoop();
  return 0;
}
// GCOVR_EXCL_STOP
