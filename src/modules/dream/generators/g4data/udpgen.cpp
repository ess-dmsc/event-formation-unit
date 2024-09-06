// Copyright (C) 2023 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate Dream udp stream from a dat file
//===----------------------------------------------------------------------===//

// GCOVR_EXCL_START

#include <modules/dream/generators/g4data/ReadoutGenerator.h>

int main(int argc, char *argv[]) {

  ReadoutGenerator DreamGen;
  uint8_t DreamDataSize = sizeof(Dream::DataParser::CDTReadout);
  DreamGen.setReadoutDataSize(DreamDataSize);

  DreamGen.argParse(argc, argv);

  DreamGen.main();
  DreamGen.transmitLoop();

  return 0;
}

// GCOVR_EXCL_STOP
