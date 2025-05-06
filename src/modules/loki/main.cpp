// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for loki
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/caen/CaenBase.h>

int main(int argc, char *argv[]) {
  MainProg Main("loki", argc, argv);

  auto Detector =
      new Caen::CaenBase(Main.DetectorSettings, DetectorType::LOKI);
  // auto Detector =
  //     new Caen::CaenBase(Main.DetectorSettings, DetectorFoo);
  // exit(-1);

  return Main.run(Detector);
}
