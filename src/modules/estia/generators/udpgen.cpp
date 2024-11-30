// Copyright (C) 2023 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial Estia readouts
//===----------------------------------------------------------------------===//

#include <modules/estia/generators/ReadoutGenerator.h>
// GCOVR_EXCL_START

int main(int argc, char *argv[]) {

  Estia::ReadoutGenerator EstiaGen;
  uint8_t EstiaDataSize = sizeof(ESSReadout::VMM3Parser::VMM3Data);
  EstiaGen.setReadoutDataSize(EstiaDataSize);

  EstiaGen.argParse(argc, argv);
  EstiaGen.main();

  EstiaGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
