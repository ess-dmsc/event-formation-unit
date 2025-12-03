// Copyright (C) 2023 - 2025 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial FREIA readouts
//===----------------------------------------------------------------------===//

// GCOVR_EXCL_START

#include <modules/freia/generators/ReadoutGenerator.h>
#include <common/readout/vmm3/VMM3Parser.h>

using namespace vmm3;
using VMM3Data = VMM3Parser::VMM3Data;

int main(int argc, char *argv[]) {

  Freia::ReadoutGenerator FreiaGen;
  uint8_t FreiaDataSize = sizeof(VMM3Data);
  FreiaGen.setReadoutDataSize(FreiaDataSize);

  FreiaGen.argParse(argc, argv);

  std::unique_ptr<FunctionGenerator> readoutTimeGenerator = 
      std::make_unique<DistributionGenerator>(FreiaGen.Settings.Frequency);
  FreiaGen.initialize(std::move(readoutTimeGenerator));

  FreiaGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
