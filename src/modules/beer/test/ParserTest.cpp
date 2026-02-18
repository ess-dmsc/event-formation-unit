// Copyright (C) 2025 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for BEER Parser - validates isValidType override
///
//===----------------------------------------------------------------------===//

#include "common/Statistics.h"
#include <common/readout/ess/Parser.h>
#include <common/testutils/TestBase.h>
#include <modules/beer/readout/Parser.h>
#include <modules/cbm/CbmTypes.h>

using namespace beer;

// clang-format off
/// \brief Valid EVENT_2D readout - should be accepted by BEER
std::vector<uint8_t> ValidEvent2DReadout {
  0x16, 0x00, 0x14, 0x00,  // Data header - Fiber 22, FEN 0, Size 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x02, 0x00, 0x00, 0x00,  // Type 0x02 (EVENT_2D), Channel 0, ADC 0
  0x64, 0x00, 0x32, 0x00   // XPos 100, YPos 50
};

/// \brief Invalid EVENT_0D readout - should be rejected by BEER
std::vector<uint8_t> InvalidEvent0DReadout {
  0x16, 0x00, 0x14, 0x00,  // Data header - Fiber 22, FEN 0, Size 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x01, 0x00, 0x00, 0x00,  // Type 0x01 (EVENT_0D), Channel 0, ADC 0
  0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
};

/// \brief Invalid IBM readout - should be rejected by BEER
std::vector<uint8_t> InvalidIBMReadout {
  0x16, 0x00, 0x14, 0x00,  // Data header - Fiber 22, FEN 0, Size 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x03, 0x00, 0x00, 0x00,  // Type 0x03 (IBM), Channel 0, ADC 0
  0xFF, 0x00, 0x00, 0x00   // NPOS 255
};
// clang-format on

class BeerParserTest : public TestBase {
protected:
  Parser BeerParser;
  Statistics Stats;
  ESSReadout::Parser::PacketDataV0 PacketData{Stats};

  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(BeerParserTest, ParseAcceptsEvent2D) {
  PacketData.DataPtr = (char *)&ValidEvent2DReadout[0];
  PacketData.DataLength = ValidEvent2DReadout.size();

  BeerParser.parse(PacketData);

  // EVENT_2D readout should be accepted
  EXPECT_EQ(BeerParser.Stats.Readouts, 1);
  EXPECT_EQ(BeerParser.Stats.ErrorType, 0);
  
  // Verify the parsed result
  EXPECT_EQ(BeerParser.Result[0].Type, static_cast<uint8_t>(cbm::CbmType::EVENT_2D));
  EXPECT_EQ(BeerParser.Result[0].Pos.XPos, 100);
  EXPECT_EQ(BeerParser.Result[0].Pos.YPos, 50);
}

TEST_F(BeerParserTest, ParserOverrideRejectsEvent0D) {
  PacketData.DataPtr = (char *)&InvalidEvent0DReadout[0];
  PacketData.DataLength = InvalidEvent0DReadout.size();

  BeerParser.parse(PacketData);

  // EVENT_0D readout should be rejected due to invalid type
  EXPECT_EQ(BeerParser.Stats.Readouts, 1);
  EXPECT_EQ(BeerParser.Stats.ErrorType, 1);
}
