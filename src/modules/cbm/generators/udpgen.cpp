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
  CbmGen.main();

  CbmGen.Settings.Type = ESSReadout::Parser::DetectorType::CBM;

  CbmGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
