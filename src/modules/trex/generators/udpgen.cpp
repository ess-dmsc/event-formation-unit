// Copyright (C) 2023 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial TREX readouts
//===----------------------------------------------------------------------===//

#include <modules/trex/generators/ReadoutGenerator.h>
// GCOVR_EXCL_START

int main(int argc, char *argv[]) {

  Trex::ReadoutGenerator TrexGen;
  uint8_t TrexDataSize = sizeof(ESSReadout::VMM3Parser::VMM3Data);
  TrexGen.setReadoutDataSize(TrexDataSize);

  TrexGen.argParse(argc, argv);
  TrexGen.main();

  TrexGen.Settings.Type = ESSReadout::Parser::DetectorType::TREX;

  TrexGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
