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

int main(int argc, char *argv[]) {

  Freia::ReadoutGenerator FreiaGen;
  uint8_t FreiaDataSize = sizeof(ESSReadout::VMM3Parser::VMM3Data);
  FreiaGen.setReadoutDataSize(FreiaDataSize);

  FreiaGen.argParse(argc, argv);

  std::shared_ptr<DistributionGenerator> distribution = DistributionGenerator::Factory(FreiaGen.Settings.Frequency);
  FreiaGen.main(distribution);

  FreiaGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
