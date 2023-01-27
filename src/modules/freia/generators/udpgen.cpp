// Copyright (C) 2023 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial FREIA readouts
//===----------------------------------------------------------------------===//

#include <modules/freia/generators/ReadoutGenerator.h>
// GCOVR_EXCL_START

int main(int argc, char *argv[]) {

  Freia::ReadoutGenerator FreiaGen;
  uint8_t FreiaDataSize = sizeof(ESSReadout::VMM3Parser::VMM3Data);
  FreiaGen.setReadoutDataSize(FreiaDataSize);

  FreiaGen.argParse(argc, argv);
  FreiaGen.main();

  FreiaGen.Settings.Type = ESSReadout::Parser::DetectorType::FREIA;

  FreiaGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
