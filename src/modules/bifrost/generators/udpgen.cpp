// Copyright (C) 2023 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate Bifrost udp stream from a dat file
//===----------------------------------------------------------------------===//

// GCOVR_EXCL_START

#include <modules/bifrost/generators/ReadoutGenerator.h>

int main(int argc, char *argv[]) {

  ReadoutGenerator BifrostGen;
  uint8_t BifrostDataSize = sizeof(Caen::DataParser::CaenReadout);
  BifrostGen.setReadoutDataSize(BifrostDataSize);

  BifrostGen.argParse(argc, argv);

  BifrostGen.main();
  BifrostGen.transmitLoop();

  return 0;
}

// GCOVR_EXCL_STOP