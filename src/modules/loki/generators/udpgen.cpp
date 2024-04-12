// Copyright (C) 2023 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generate artificial LOKI readouts
//===----------------------------------------------------------------------===//

#include <modules/loki/generators/LokiReadoutGenerator.h>
// GCOVR_EXCL_START

int main(int argc, char *argv[]) {

  Caen::LokiReadoutGenerator LokiGen;
  uint8_t LokiDataSize = sizeof(Caen::DataParser::CaenReadout);
  LokiGen.setReadoutDataSize(LokiDataSize);

  LokiGen.argParse(argc, argv);
  LokiGen.main();

  LokiGen.transmitLoop();

  return 0;
}
// GCOVR_EXCL_STOP
