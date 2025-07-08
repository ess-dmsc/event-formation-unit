// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <caen/CaenCounters.h>
#include <caen/CaenInstrument.h>
#include <caen/generators/ReadoutGenerator.h>
#include <caen/readout/DataParser.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/SocketMock.h>
#include <common/testutils/TestBase.h>
#include <generators/functiongenerators/DistributionGenerator.h>
#include <gtest/gtest.h>

using namespace Caen;

class AmplitudeMaskTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

// helper to execute the common test logic with detector parameter
static void runMaskTest(uint16_t maskTrunc, DetectorType detector) {
  MultipackageSocketMock socket{};
  const std::chrono::microseconds pulseTimeDuration{71428};
  auto readoutTimeGenerator = std::make_unique<DistributionGenerator>(
      ReadoutGeneratorBase::DEFAULT_FREQUENCY);

  ReadoutGenerator gen;
  gen.Settings.headerVersion = 0;
  gen.Settings.Detector = detector;
  gen.Settings.FiberMask = 0x3f;
  gen.CaenSettings.AmplitudeMask = maskTrunc;
  gen.CaenSettings.FENMask = 0x1;
  gen.CaenSettings.GroupMask = 0x7fff;
  gen.CaenSettings.GroupVals = 15;

  // Set LOKI flag based on detector type
  gen.CaenSettings.Loki = (detector == DetectorType::LOKI);

  gen.setReadoutDataSize(sizeof(DataParser::CaenReadout));
  gen.initialize(std::move(readoutTimeGenerator));

  socket.Clear();
  gen.generatePackets(&socket, pulseTimeDuration);
  auto collection = socket.PackageList();
  ASSERT_GT(collection.size(), 1);

  for (auto &buffer : collection) {
    // Caen readout size = 24. Header size is 30
    // MaxBuffer = 8972
    // Readout count = (8972 - 30) / 24 round down => 372
    // One packet for caen will only use 8958 bytes if filled
    int ReadoutCount = 372;
    ASSERT_EQ(buffer.size(), 8958);
    auto *data = buffer.data();
    // Set data pointer to readouts, skipping the header
    auto *readoutData = data + sizeof(ESSReadout::Parser::PacketHeaderV0);

    DataParser Parser;
    auto Res = Parser.parse(readoutData,
                            buffer.size() -
                                sizeof(ESSReadout::Parser::PacketHeaderV0));
    ASSERT_EQ(Res, ReadoutCount);
    ASSERT_EQ(Parser.Stats.Readouts, ReadoutCount);
    ASSERT_EQ(Parser.Stats.DataHeaders, ReadoutCount);
    ASSERT_EQ(Parser.Stats.DataHeaderSizeErrors, 0);
    ASSERT_EQ(Parser.Stats.RingFenErrors, 0);
    ASSERT_EQ(Parser.Stats.DataLenMismatch, 0);
    ASSERT_EQ(Parser.Stats.DataLenInvalid, 0);
    ASSERT_EQ(Parser.Result.size(), ReadoutCount);

    for (auto &rd : Parser.Result) {
      // All detector types have AmpA and AmpB
      ASSERT_LE(rd.AmpA, maskTrunc);
      ASSERT_LE(rd.AmpB, maskTrunc);

      if (detector == DetectorType::LOKI) {
        // LOKI uses all four amplitude channels
        ASSERT_LE(rd.AmpC, maskTrunc);
        ASSERT_LE(rd.AmpD, maskTrunc);
      } else {
        // Other detectors should have AmpC and AmpD set to 0
        ASSERT_EQ(rd.AmpC, 0);
        ASSERT_EQ(rd.AmpD, 0);
      }
    }
  }
}

// BIFROST tests
TEST_F(AmplitudeMaskTest, BifrostMaskZero) {
  runMaskTest(0u, DetectorType::BIFROST);
}

TEST_F(AmplitudeMaskTest, BifrostMaskSmallF) {
  runMaskTest(0xFu, DetectorType::BIFROST);
}

TEST_F(AmplitudeMaskTest, BifrostMaskFFFF) {
  runMaskTest(0xFFFFu, DetectorType::BIFROST);
}

TEST_F(AmplitudeMaskTest, MaskInputOverflow) {
  runMaskTest(static_cast<uint16_t>(0xFFFFFFu), DetectorType::BIFROST);
}

// LOKI tests
TEST_F(AmplitudeMaskTest, LokiMaskZero) { runMaskTest(0u, DetectorType::LOKI); }

TEST_F(AmplitudeMaskTest, LokiMaskSmallF) {
  runMaskTest(0xFu, DetectorType::LOKI);
}

TEST_F(AmplitudeMaskTest, LokiMaskFFFF) {
  runMaskTest(0xFFFFu, DetectorType::LOKI);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
