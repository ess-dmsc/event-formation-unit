// Copyright (C) 2023 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial TTLMON readouts
//===----------------------------------------------------------------------===//

#include <modules/cbm/generators/ReadoutGenerator.h>
#include <modules/cbm/geometry/Parser.h>
// GCOVR_EXCL_START

int main(int argc, char *argv[]) {

  cbm::ReadoutGenerator TTLMonGen;
  uint8_t TTLMonDataSize = sizeof(cbm::Parser::Data);
  TTLMonGen.setReadoutDataSize(TTLMonDataSize);

  TTLMonGen.argParse(argc, argv);
  TTLMonGen.main();

  TTLMonGen.Settings.Type = ESSReadout::Parser::DetectorType::CBM;

  TTLMonGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
