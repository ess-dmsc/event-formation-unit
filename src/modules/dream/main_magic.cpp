// Copyright (C) 2023 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for magic
///
/// DREAM and MAGIC implementations are identical and the configuration
/// file determines which geometry to use.
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/dream/DreamBase.h>

int main(int argc, char *argv[]) {
  MainProg Main("magic", argc, argv);

  auto Detector = new Dream::DreamBase(Main.DetectorSettings, DetectorType::MAGIC);

  return Main.run(Detector);
}
