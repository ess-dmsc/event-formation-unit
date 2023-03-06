// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for Timepix3
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/timepix3/Timepix3Base.h>

int main(int argc, char *argv[]) {
  MainProg Main("timepix3", argc, argv);

  auto Detector = new Timepix3::Timepix3Base(Main.DetectorSettings);

  return Main.run(Detector);
}
