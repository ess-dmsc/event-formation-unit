// Copyright (C) 2024 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial BEER readouts
//===----------------------------------------------------------------------====//

// GCOVR_EXCL_START

#include <generators/functiongenerators/DistributionGenerator.h>
#include <modules/beer/generators/ReadoutGenerator.h>
#include <modules/cbm/readout/Parser.h>

int main(int argc, char *argv[]) {

  beer::ReadoutGenerator BeerGen;
  uint8_t cbmReadoutDataSize = sizeof(cbm::Parser::CbmReadout);
  BeerGen.setReadoutDataSize(cbmReadoutDataSize);

  BeerGen.argParse(argc, argv);

  BeerGen.initialize(std::make_unique<DistributionGenerator>(BeerGen.Settings.Frequency));

  BeerGen.transmitLoop();
  return 0;
}
// GCOVR_EXCL_STOP
