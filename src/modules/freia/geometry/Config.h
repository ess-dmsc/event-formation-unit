// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Get detector configuration from json file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Trace.h>
#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Freia {

class Config {
public:
  static const unsigned int VMMsPerFEN{4};
  static const unsigned int CassettesPerFEN{2};
  static const unsigned int NumWiresPerCassette{32};
  static const unsigned int NumStripsPerCassette{64};

  Config() {};

  Config(std::string ConfigFile);

  std::vector<uint16_t> NumFens;   // # FENs per ring
  std::vector<uint16_t> FENOffset; // Global FEN offset per ring
  std::vector<uint16_t> VMMOffset; // Global VMM offset per ring

  unsigned int NumRings{0};
  uint32_t NumPixels{0};
  uint32_t NumCassettes{0};

};

} // namespace Freia