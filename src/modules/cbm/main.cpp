// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for Common Beam Monitor (CBM) detector module.
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/cbm/CbmBase.h>

int main(int argc, char *argv[]) {
  MainProg Main(DetectorType::CBM, argc, argv);

  auto Detector = new cbm::CbmBase(Main.DetectorSettings);

  return Main.run(Detector);
}
