
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
#include <memory>

using namespace ESSReadout;

class TestHeaderFactory {

  std::unique_ptr<Parser::PacketHeader> myHeaderPtr;

public:
  Parser::PacketHeader *createHeader(const Parser::HeaderVersion &version) {
    if (version == Parser::HeaderVersion::V0) {
      Parser::PacketHeaderV0 *header = new Parser::PacketHeaderV0();
      // memset(header, 0, sizeof(Parser::PacketHeaderV0));

      myHeaderPtr =
          std::make_unique<Parser::PacketHeader>(Parser::PacketHeader(header));
      return myHeaderPtr.get();

    } else if (version == Parser::HeaderVersion::V1) {
      Parser::PacketHeaderV1 *header = new Parser::PacketHeaderV1();
      // memset(header, 0, sizeof(Parser::PacketHeaderV1));

      myHeaderPtr =
          std::make_unique<Parser::PacketHeader>(Parser::PacketHeader(header));
      return myHeaderPtr.get();

    } else {
      return nullptr;
    }
  }

  Parser::PacketHeader *createHeader(Parser::PacketHeaderV0 &header) {
    myHeaderPtr =
        std::make_unique<Parser::PacketHeader>(Parser::PacketHeader(&header));
    return myHeaderPtr.get();
  }
};