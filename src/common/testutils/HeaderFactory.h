
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
  std::unique_ptr<Parser::PacketHeaderV0> headerV0;
  std::unique_ptr<Parser::PacketHeaderV1> headerV1;

public:
  Parser::PacketHeader &createHeader(const Parser::HeaderVersion &version) {
    if (version == Parser::HeaderVersion::V1) {
      headerV1 = std::make_unique<Parser::PacketHeaderV1>(Parser::PacketHeaderV1());

      myHeaderPtr =
          std::make_unique<Parser::PacketHeader>(Parser::PacketHeader(headerV1.get()));
      return *myHeaderPtr;

    } else {
      headerV0 = std::make_unique<Parser::PacketHeaderV0>(Parser::PacketHeaderV0());

      myHeaderPtr =
          std::make_unique<Parser::PacketHeader>(Parser::PacketHeader(headerV0.get()));
      return *myHeaderPtr;
    }
  }

  Parser::PacketHeader &createHeader(Parser::PacketHeaderV0 &header) {
    myHeaderPtr =
        std::make_unique<Parser::PacketHeader>(Parser::PacketHeader(&header));
    return *myHeaderPtr;
  }
};