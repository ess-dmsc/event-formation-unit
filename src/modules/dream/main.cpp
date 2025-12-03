// Copyright (C) 2022 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for dream
//===----------------------------------------------------------------------===//

#include <common/types/DetectorType.h>
#include <efu/MainProg.h>
#include <modules/dream/DreamBase.h>

int main(int argc, char *argv[]) {
  MainProg Main("dream", argc, argv);

  auto Detector =
      new Dream::DreamBase<DetectorType::DREAM>(Main.DetectorSettings);

  return Main.run(Detector);
}
