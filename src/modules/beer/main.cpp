// Copyright (C) 2022 - 2026 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// @file
/// @brief Main entry for BEER detector module.
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/beer/BeerBase.h>

int main(int argc, char *argv[]) {
  MainProg Main(DetectorType::BEER, argc, argv);

  auto Detector = new beer::BeerBase(Main.DetectorSettings);

  return Main.run(Detector);
}
