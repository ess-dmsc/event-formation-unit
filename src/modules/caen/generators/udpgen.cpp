// Copyright (C) 2024 - 2025 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial CAEN readouts
//===----------------------------------------------------------------------===//

#include <generators/functiongenerators/DistributionGenerator.h>
#include <modules/caen/generators/ReadoutGenerator.h>
#include <utility>
// GCOVR_EXCL_START

int main(int argc, char *argv[]) {
  Caen::ReadoutGenerator CaenGen;
  CaenGen.setReadoutDataSize(sizeof(Caen::DataParser::CaenReadout));
  CaenGen.argParse(argc, argv);

  auto readoutTimeGenerator =
      std::make_unique<DistributionGenerator>(CaenGen.Settings.Frequency);
  
  // Move the ownership of the readout time generator to the CaenGen instance
  CaenGen.initialize(std::move(readoutTimeGenerator));
  CaenGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
