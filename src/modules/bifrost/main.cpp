// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for caen
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/caen/CaenBase.h>

int main(int argc, char * argv[]) {
  MainProg Main("bifrost", argc, argv);

  auto Detector = new Caen::CaenBase(Main.DetectorSettings);

  return Main.run(Detector);
}
