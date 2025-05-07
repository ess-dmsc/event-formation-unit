// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for CSPEC
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/caen/CaenBase.h>

int main(int argc, char *argv[]) {
  MainProg Main("cspec", argc, argv);

  auto Detector =
      new Caen::CaenBase(Main.DetectorSettings, DetectorType::CSPEC);

  return Main.run(Detector);
}
