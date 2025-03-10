// Copyright (C) 2025 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3 readouts for multi-blade based setups
//===----------------------------------------------------------------------===//

// GCOVR_EXCL_START

#include <modules/freia/generators/MultiBladeGenerator.h>
#include <common/readout/vmm3/VMM3Parser.h>

int main(int argc, char *argv[]) {

  Freia::MultiBladeGenerator Generator;
  uint8_t MultiBladeDataSize = sizeof(ESSReadout::VMM3Parser::VMM3Data);
  Generator.setReadoutDataSize(MultiBladeDataSize);

  Generator.argParse(argc, argv);
  Generator.main();

  Generator.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
