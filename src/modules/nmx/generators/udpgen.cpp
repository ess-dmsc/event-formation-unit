// Copyright (C) 2023 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial NMX readouts
//===----------------------------------------------------------------------===//

#include <modules/nmx/generators/ReadoutGenerator.h>
// GCOVR_EXCL_START

int main(int argc, char *argv[]) {

  Nmx::ReadoutGenerator NmxGen;
  uint8_t DataSize = sizeof(ESSReadout::VMM3Parser::VMM3Data);
  NmxGen.setReadoutDataSize(DataSize);

  NmxGen.argParse(argc, argv);
  NmxGen.main();

  NmxGen.Settings.Type = ESSReadout::Parser::DetectorType::NMX;

  NmxGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
