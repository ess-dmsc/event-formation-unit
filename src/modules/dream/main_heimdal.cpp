// Copyright (C) 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for heimdal
///
/// DREAM, MAGIC and HEIMDAL implementations are identical and the configuration
/// file determines which geometry to use.
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/dream/DreamBase.h>

int main(int argc, char *argv[]) {
  MainProg Main("heimdal", argc, argv);

  auto Detector = new Dream::DreamBase(Main.DetectorSettings);

  return Main.run(Detector);
}
