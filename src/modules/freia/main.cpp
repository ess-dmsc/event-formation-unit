// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for freia
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/freia/FreiaBase.h>

int main(int argc, char * argv[]) {
  MainProg Main("freia", argc, argv);

  auto Detector = new Freia::FreiaBase(Main.DetectorSettings);

  return Main.run(Detector);
}
