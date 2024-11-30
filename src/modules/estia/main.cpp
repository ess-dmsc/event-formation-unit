// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for estia
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/estia/EstiaBase.h>

int main(int argc, char *argv[]) {
  MainProg Main("estia", argc, argv);

  auto Detector = new Estia::EstiaBase(Main.DetectorSettings);

  return Main.run(Detector);
}
