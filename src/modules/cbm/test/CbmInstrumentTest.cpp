// Copyright (C) 2021 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/kafka/EV44Serializer.h>
#include <common/reduction/Event.h>
#include <common/testutils/HeaderFactory.h>
#include <common/testutils/TestBase.h>
#include <memory>
#include <modules/cbm/CbmInstrument.h>

using namespace cbm;
using namespace ESSReadout;

// clang-format off

std::vector<uint8_t> ValidMonitorReadouts {
  0x16, 0x00, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0
};

/// \brief Monitor readout with invalid Ring
std::vector<uint8_t> RingNotInCfgReadout {
  0x12, 0x00, 0x14, 0x00,  // Fiber 18, FEN 0, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0
};

/// \brief Monitor readout with invalid FEN and Channel
std::vector<uint8_t> FenAndChannelNotInCfgReadout {
  // Invalid FEN - not in configuration
  0x16, 0x06, 0x14, 0x00,  // Fiber 22, FEN 6, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00,   // XPos 0, YPos 0

  // Invalid Channel - not in configuration
  0x16, 0x06, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 14, ADC 1
  0x00, 0x00, 0x00, 0x00,   // XPos 0, YPos 0

  // Invalid Ring and Channel - not in configuration
  0x16, 0x06, 0x14, 0x00,  // Fiber 22, FEN 6, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 14, ADC 1
  0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
};

std::vector<uint8_t> MonitorReadoutTOF {
  // First monitor readout - Negative PrevTOF - possibly unreachable!
  0x16, 0x00, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x01, 0x00, 0x00, 0x00,  // Type 1, Ch 0, ADC 0
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  // Second monitor readout - Negative TOF, positive PrevTOF
  0x16, 0x00, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x01, 0x00, 0x00, 0x00,  // Type 1, Ch 0, ADC 0
  0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
};
// clang-format on

class CbmInstrumentTest : public TestBase {
public:
protected:
  struct Counters counters;
  BaseSettings Settings;
  Config Configuration;
  HashMap2D<EV44Serializer> EV44SerializerPtrs{11};
  HashMap2D<fbserializer::HistogramSerializer<int32_t>> HistogramSerializerPtrs{
      11};
  CbmInstrument *cbm;
  std::unique_ptr<TestHeaderFactory> headerFactory;
  Event TestEvent;           // used for testing generateEvents()
  std::vector<Event> Events; // used for testing generateEvents()

  void SetUp() override {
    Settings.ConfigFile = CBM_CONFIG;

    Configuration = Config(Settings.ConfigFile);
    Configuration.loadAndApply();

    std::unique_ptr<EV44Serializer> ev44Serializer =
        std::make_unique<EV44Serializer>(115000, "cbm");
    EV44SerializerPtrs.add(0, 0, ev44Serializer);
    counters = {};

    headerFactory = std::make_unique<TestHeaderFactory>();
    cbm = new CbmInstrument(counters, Configuration, EV44SerializerPtrs,
                            HistogramSerializerPtrs);
    cbm->ESSReadoutParser.Packet.HeaderPtr =
        headerFactory->createHeader(ESSReadout::Parser::V1);
  }
  void TearDown() override {}

  void makeHeader(ESSReadout::Parser::PacketDataV0 &Packet,
                  std::vector<uint8_t> &testdata) {
    Packet.HeaderPtr = headerFactory->createHeader(ESSReadout::Parser::V1);
    Packet.DataPtr = (char *)&testdata[0];
    Packet.DataLength = testdata.size();
    Packet.Time.setReference(ESSTime(0, 0));
    Packet.Time.setPrevReference(ESSTime(0, 0));
  }
};

// Test cases below
TEST_F(CbmInstrumentTest, Constructor) { ASSERT_EQ(counters.RingCfgError, 0); }

TEST_F(CbmInstrumentTest, TestReadoutProcessingType1) {
  makeHeader(cbm->ESSReadoutParser.Packet, ValidMonitorReadouts);

  cbm->CbmParser.parse(cbm->ESSReadoutParser.Packet);
  counters.CbmStats = cbm->CbmParser.Stats;

  EXPECT_EQ(counters.CbmStats.Readouts, 1);
  EXPECT_EQ(counters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(counters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(counters.CbmStats.ErrorADC, 0);
  EXPECT_EQ(counters.CbmStats.ErrorType, 0);

  cbm->processMonitorReadouts();
  EXPECT_EQ(counters.RingCfgError, 0);
  EXPECT_EQ(counters.CbmCounts, 1);
  EXPECT_EQ(counters.NoSerializerCfgError, 0);
}

TEST_F(CbmInstrumentTest, RingConfigurationError) {
  makeHeader(cbm->ESSReadoutParser.Packet, RingNotInCfgReadout);

  cbm->CbmParser.parse(cbm->ESSReadoutParser.Packet);
  counters.CbmStats = cbm->CbmParser.Stats;

  EXPECT_EQ(counters.CbmStats.Readouts, 1);
  EXPECT_EQ(counters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(counters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(counters.CbmStats.ErrorADC, 0);
  EXPECT_EQ(counters.CbmStats.ErrorType, 0);

  cbm->processMonitorReadouts();
  EXPECT_EQ(counters.RingCfgError, 1);
  EXPECT_EQ(counters.CbmCounts, 0);
  EXPECT_EQ(counters.NoSerializerCfgError, 0);
}

TEST_F(CbmInstrumentTest, NoSerializerCfgError) {
  makeHeader(cbm->ESSReadoutParser.Packet, FenAndChannelNotInCfgReadout);

  cbm->CbmParser.parse(cbm->ESSReadoutParser.Packet);
  counters.CbmStats = cbm->CbmParser.Stats;

  EXPECT_EQ(counters.CbmStats.Readouts, 3);
  EXPECT_EQ(counters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(counters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(counters.CbmStats.ErrorADC, 0);
  EXPECT_EQ(counters.CbmStats.ErrorType, 0);

  cbm->processMonitorReadouts();
  EXPECT_EQ(counters.RingCfgError, 0);
  EXPECT_EQ(counters.CbmCounts, 0);
  EXPECT_EQ(counters.NoSerializerCfgError, 3);
}

TEST_F(CbmInstrumentTest, BeamMonitorTOF) {
  makeHeader(cbm->ESSReadoutParser.Packet, MonitorReadoutTOF);
  cbm->ESSReadoutParser.Packet.Time.setReference(ESSTime(1, 100000));
  cbm->ESSReadoutParser.Packet.Time.setPrevReference(ESSTime(1, 0));

  cbm->CbmParser.parse(cbm->ESSReadoutParser.Packet);
  counters.CbmStats = cbm->CbmParser.Stats;

  cbm->processMonitorReadouts();
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
