// Copyright (C) 2023 - 2025 European Spallation Source ERIC
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

  std::unique_ptr<FunctionGenerator> readoutTimeGenerator = 
      std::make_unique<DistributionGenerator>(TrexGen.Settings.Frequency);
  TrexGen.initialize(std::move(readoutTimeGenerator));

  TrexGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
