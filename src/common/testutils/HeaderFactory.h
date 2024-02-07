
// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Test helper to Build ESS headers
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/readout/ess/Parser.h>

using namespace ESSReadout;

class TestHeaderFactory {

  Parser::PacketHeader myHeader;

public:
  Parser::PacketHeader *createHeader(const Parser::HeaderVersion &version) {
    if (version == Parser::HeaderVersion::V0) {
      Parser::PacketHeaderV0 *header{nullptr};
      memset(header, 0, sizeof(Parser::PacketHeaderV0));

      myHeader = Parser::PacketHeader(header);
      return &myHeader;

    } else if (version == Parser::HeaderVersion::V1) {
      Parser::PacketHeaderV0 *header{nullptr};
      memset(header, 0, sizeof(Parser::PacketHeaderV1));

      myHeader = Parser::PacketHeader(header);
      return &myHeader;

    } else {
      return nullptr;
    }
  }
};