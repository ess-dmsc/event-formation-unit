// Copyright (C) 2023 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial DREAM readouts
//===----------------------------------------------------------------------===//

#include <modules/dream/generators/ReadoutGenerator.h>
// GCOVR_EXCL_START

int main(int argc, char *argv[]) {

  Dream::DreamReadoutGenerator DreamGen;
  uint8_t DreamDataSize = sizeof(Dream::DataParser::DreamReadout);
  DreamGen.setReadoutDataSize(DreamDataSize);

  DreamGen.argParse(argc, argv);
  DreamGen.main();

  DreamGen.Settings.Type = ESSReadout::Parser::DetectorType::DREAM;

  DreamGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
