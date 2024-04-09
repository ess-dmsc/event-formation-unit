// Copyright (C) 2021 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/HeaderFactory.h>
#include <common/kafka/EV44Serializer.h>
#include <common/readout/ess/Parser.h>
#include <common/reduction/Event.h>
#include <common/testutils/TestBase.h>
#include <cbm/CbmInstrument.h>

using namespace cbm;
using namespace ESSReadout;

// clang-format off

std::vector<uint8_t> MonitorReadout {
  // Errors caught when parsing readouts

  // First monitor readout - Valid
  0x16, 0x00, 0x14, 0x00,  // Data Header - Fiber 22, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  // Second monitor readout - invalid Ring
  0x18, 0x00, 0x14, 0x00,  // Data Header, Fiber 24, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  // Third monitor readout - invalid FEN
  0x17, 0x22, 0x14, 0x00,  // Data Header, Fiber 23, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  // Fourth monitor readout - invalid Channel
  0x17, 0x00, 0x14, 0x00,  // Data Header, Fiber 23, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x01, 0x07, 0x01, 0x00,  // Type 1, Ch 7, ADC 1
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  // Fifth monitor readout - invalid ADC
  0x17, 0x00, 0x14, 0x00,  // Data Header, Fiber 23, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x01, 0x00, 0x00, 0x00,  // Type 1, Ch 0, ADC 0
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  // Errors caught when processing readouts

  // Sixth monitor readout - invalid RingCfg
  0x12, 0x00, 0x14, 0x00,  // Data Header, Fiber 18, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  // Seventh monitor readout - invalid FENCfg
  0x17, 0x01, 0x14, 0x00,  // Data Header, Fiber 18, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
};


std::vector<uint8_t> MonitorReadoutTOF {
  // First monitor readout - Negative PrevTOF - possibly unreachable!
  0x16, 0x00, 0x14, 0x00,  // Data Header - Fiber 22, FEN 0
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x01, 0x00, 0x00, 0x00,  // Type 1, Ch 0, ADC 0
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  // Second monitor readout - Negative TOF, positive PrevTOF
  0x16, 0x00, 0x14, 0x00,  // Data Header - Fiber 22, FEN 0
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
  std::vector<std::unique_ptr<EV44Serializer>> serializers;
  CbmInstrument *cbm;
  std::unique_ptr<TestHeaderFactory> headerFactory;
  Event TestEvent;           // used for testing generateEvents()
  std::vector<Event> Events; // used for testing generateEvents()

  void SetUp() override {
    Settings.ConfigFile = CBM_CONFIG;
    serializers.push_back(std::make_unique<EV44Serializer>(115000, "cbm"));
    counters = {};

    headerFactory = std::make_unique<TestHeaderFactory>();
    cbm = new CbmInstrument(counters, Settings);
    cbm->SerializersPtr.push_back(serializers[0].get());
    cbm->ESSReadoutParser.Packet.HeaderPtr = headerFactory->createHeader(ESSReadout::Parser::V1);
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
TEST_F(CbmInstrumentTest, Constructor) {
  ASSERT_EQ(counters.RingCfgErrors, 0);
  ASSERT_EQ(counters.FENCfgErrors, 0);
}

TEST_F(CbmInstrumentTest, BeamMonitor) {
  makeHeader(cbm->ESSReadoutParser.Packet, MonitorReadout);

  cbm->CbmParser.parse(cbm->ESSReadoutParser.Packet);
  counters.CbmStats = cbm->CbmParser.Stats;

  ASSERT_EQ(counters.CbmStats.Readouts, 7);
  ASSERT_EQ(counters.CbmStats.ErrorFiber, 1);
  ASSERT_EQ(counters.CbmStats.ErrorFEN, 1);
  ASSERT_EQ(counters.CbmStats.ErrorADC, 1);

  cbm->processMonitorReadouts();
  ASSERT_EQ(counters.RingCfgErrors, 1);
  ASSERT_EQ(counters.FENCfgErrors, 1);
  ASSERT_EQ(counters.MonitorCounts, 2);

  ASSERT_EQ(counters.FENCfgErrors, 1);
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
