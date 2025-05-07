// Copyright (C) 2024 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial CAEN readouts
//===----------------------------------------------------------------------===//

#include <modules/caen/generators/ReadoutGenerator.h>
// GCOVR_EXCL_START

int main(int argc, char *argv[]) {
  Caen::ReadoutGenerator CaenGen;
  CaenGen.argParse(argc, argv);

  CaenGen.main();
  CaenGen.setReadoutDataSize(sizeof(Caen::DataParser::CaenReadout));
  CaenGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
