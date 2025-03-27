// Copyright (C) 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Main entry for TBL-MB (multi-blade test-beam-line)
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/freia/FreiaBase.h>

int main(int argc, char *argv[]) {
  MainProg Main("tblmb", argc, argv);

  auto Detector = new Freia::FreiaBase(Main.DetectorSettings);

  return Main.run(Detector);
}
