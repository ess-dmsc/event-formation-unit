// Copyright (C) 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for caen
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/caen/CaenBase.h>

int main(int argc, char *argv[]) {
  MainProg Main("tbl3he", argc, argv);

  auto Detector =
      new Caen::CaenBase(Main.DetectorSettings, DetectorType::TBL3HE);

  return Main.run(Detector);
}
