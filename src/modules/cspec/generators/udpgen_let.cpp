// Copyright (C) 2023 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial LET readouts
//===----------------------------------------------------------------------===//

#include <modules/cspec/generators/LETReadoutGenerator.h>
// GCOVR_EXCL_START

int main(int argc, char *argv[]) {

  Cspec::LETReadoutGenerator CspecGen;
  uint8_t CspecDataSize = sizeof(ESSReadout::VMM3Parser::VMM3Data);
  CspecGen.setReadoutDataSize(CspecDataSize);

  CspecGen.argParse(argc, argv);
  CspecGen.main();

  CspecGen.Settings.Type = ESSReadout::Parser::DetectorType::CSPEC;

  CspecGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
