// Copyright (C) 2023 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate Bifrost udp stream from a dat file
//===----------------------------------------------------------------------===//

#include <modules/bifrost/generators/ReadoutGenerator.h>

int main(int argc, char *argv[]) {

  ReadoutGenerator BifrostGen;
  uint8_t BifrostDataSize = sizeof(Caen::DataParser::CaenReadout);
  BifrostGen.setReadoutDataSize(BifrostDataSize);

  BifrostGen.argParse(argc, argv);
  BifrostGen.Settings.Type = ESSReadout::Parser::DetectorType::BIFROST;

  BifrostGen.main();
  BifrostGen.transmitLoop();

  return 0;
}
