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

  // If parse come back with -1 (help), the program will exit
  if (!BifrostGen.argParse(argc, argv)) {
    return 0;
  }

  BifrostGen.Settings.Type = ESSReadout::Parser::DetectorType::BIFROST;

  BifrostGen.main();
  BifrostGen.transmitLoop();

  return 0;
}

// GCOVR_EXCL_STOP